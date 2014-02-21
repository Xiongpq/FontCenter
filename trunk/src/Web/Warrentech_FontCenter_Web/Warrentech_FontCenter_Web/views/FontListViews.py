#!/usr/bin/env python
# coding=utf-8
from django.http import HttpResponse
from Warrentech_FontCenter_Web.models.FontModels import Font
from Warrentech_FontCenter_Web.models.MissingLog import MissingLog
from django.shortcuts import render
import os.path
from struct import Struct, unpack_from
from hashlib import md5
from zipfile import ZipFile, ZIP_DEFLATED, is_zipfile
from django.contrib.auth.decorators import login_required
import re
from django.db.models.query_utils import Q
from django.utils import simplejson
from urllib import unquote

global isReleaseVersion
isReleaseVersion = 'SERVER_SOFTWARE' in os.environ
SYSTEM_ENCODING = isReleaseVersion and 'utf8' or 'gbk'

import logging
import pybcs

BCS_HOST = 'bcs.duapp.com'
AK = 'kcuOUo51MNqEwAFGEVOGxvWy'
SK = 'AuDUW107HmqOhDFzMqHMfEtFeshyZcmB'

global baebcs
baebcs = pybcs.BCS(BCS_HOST, AK, SK, pybcs.HttplibHTTPC)

SYS_UPLOAD_DIRS = os.path.join(os.path.dirname(__file__), '../upload_files/sys/').replace('\\', '/')
CAD_UPLOAD_DIRS = os.path.join(os.path.dirname(__file__), '../upload_files/cad/').replace('\\', '/')
SHX_EXT = u'.shx'
CAD_FONT = u'cad'
SYS_EXT = [u'.ttf', u'.otf']

def log(message):
    with open(os.path.join(os.path.dirname(__file__), '../static/exception.log').replace('\\', '/'),'a') as file:
        file.writelines(message + '\n')

def upload_font(request, type):
    if request.method == 'POST':
        _file = request.FILES['file']
        if type == CAD_FONT:
            _handle_uploaded_file(_file, False)
        else:
            _handle_uploaded_file(_file, True)
    return HttpResponse(True)

def _handle_uploaded_file(f, is_sys):
    if is_sys == True:
        file_path = SYS_UPLOAD_DIRS
    else:
        file_path = CAD_UPLOAD_DIRS
    try:
        m = md5()
        for chunk in f.chunks():
            m.update(chunk)
        file_name_md5 = m.hexdigest()
        file_full_path = file_path + '/' + file_name_md5
        if not os.path.exists(file_full_path):
            with open(file_full_path, 'wb') as destination:
                for chunk in f.chunks():
                    destination.write(chunk)
    finally:
        if not is_zipfile(file_full_path):
            _save_font_info(file_path, file_name_md5, f.name, is_sys)
        else:
            try:
                with ZipFile(file_full_path, 'r') as font_zip:
                    i = 0
                    for name in font_zip.namelist():
                        i+=1
                        try:
                            font_content = font_zip.read(name)
                            try:
                                name = name.decode('gbk')
                            except:
                                pass
                            m = md5()
                            m.update(font_content)
                            font_file_name_md5 = m.hexdigest()
                            font_file_full_path = file_path + '/' + font_file_name_md5
                            with open(font_file_full_path, 'wb') as destination:
                                destination.write(font_content)
                        except:
                            print name, font_file_name_md5
                        else:
                            _save_font_info(file_path, font_file_name_md5, name, is_sys)
            finally:
                os.remove(file_full_path)

def _save_font_info(file_path, file_name_md5, font_file_name, is_sys):
    _font_file_name,_font_file_ext = os.path.splitext(font_file_name)
    if is_sys:
        fonts = Font.objects.filter(file_hash=file_name_md5, sys_font=is_sys)
    else:
        fonts = Font.objects.filter(file_hash=file_name_md5, postscript_name=_font_file_name, sys_font=is_sys)
    if len(fonts) == 0:
        if is_sys:
            font_names = _get_font_name(file_path + '/' + file_name_md5)
            font_names.update({'sys_font': True})
        else:
            font_names = dict(family_name='', full_name='', postscript_name=_font_file_name, sys_font=False)

        if len(font_names) == 0:
            return
        try:
            Font.objects.create(file_ext=_font_file_ext.lower(), family_name = font_names['family_name'], full_name = font_names['full_name'], postscript_name = font_names['postscript_name'] ,file_hash = file_name_md5, sys_font = font_names['sys_font'])    
            MissingLog.objects.filter(sys_font=is_sys,font_name=is_sys and font_names['postscript_name'] or (_font_file_name + _font_file_ext)).delete()
            if is_sys:
                temp_zip_file = _generate_zip_file(SYS_UPLOAD_DIRS + file_name_md5, _font_file_name + _font_file_ext.lower())
                obj_name = '/sys/%s.zip' % font_names['postscript_name']
            else:
                temp_zip_file = _generate_zip_file(CAD_UPLOAD_DIRS + file_name_md5, _font_file_name + _font_file_ext.lower())
                obj_name = '/cad/%s%s.zip' % (_font_file_name, _font_file_ext)
            b = baebcs.bucket('fontcenter')
            o = b.object(obj_name.lower().encode('utf8'))
            o.put_file(temp_zip_file.encode('utf8'))
        except Exception, e:
            print list(e)
            print font_names
    else:
        print file_name_md5

@login_required
def font_list(request, keyword):
    if keyword == False:
        keyword = ''
    fonts = Font.objects.filter(family_name__icontains=keyword)
    return render(request, 'FontList/list.html', locals())

def get_font(request,type, keyword):
    try:
        _base_name, _ext_name = os.path.splitext(keyword.encode())
        _base_name = unquote(_base_name).decode()
        if type == CAD_FONT:
            result = Font.objects.get(Q(postscript_name=_base_name) & Q(file_ext=_ext_name))
        else:
            result = Font.objects.get(Q(family_name=_base_name) | Q(postscript_name=_base_name))
        return download(request, type, result.file_hash, result.postscript_name + result.file_ext)
    except Exception, e:
        try:
            MissingLog.objects.get(font_name= keyword, sys_font= type != CAD_FONT)
        except:
            MissingLog.objects.create(font_name= keyword, sys_font= type != CAD_FONT)
        return HttpResponse(simplejson.dumps(False))

def download_local(request, type, md5, file_name):
    
    file_name = _filter_file_name(file_name)

    file_name_without_ext, file_ext = os.path.splitext(file_name)

    if type == CAD_FONT:
        file_path = CAD_UPLOAD_DIRS + md5
    else:
        file_path = SYS_UPLOAD_DIRS + md5
    zip_file_path = '%s_%s.zip' % (file_path, file_name_without_ext)    

    try:
        if not os.path.exists(zip_file_path):
            _generate_zip_file(file_path , file_name)

        font_data = open(zip_file_path , 'rb').read()
        response = HttpResponse(font_data, mimetype='application/octet-stream')
        response['Content-Disposition'] = 'attachment; filename=' + file_name_without_ext.encode('utf-8') + '.zip'
        return response
    except Exception, e:
        return HttpResponse(e.strerror)

def download(request, type, md5, file_name):

    if type == CAD_FONT:
        file_path = 'cad'
    else:
        file_path = 'sys'
    
    obj_name = '/%s/%s.zip' % (file_path, file_name)

    try:
        if isReleaseVersion:
            error_code, font_data = baebcs.get_object('fontcenter', obj_name.lower().encode('utf8'))
        if error_code == 0:
            response = HttpResponse(font_data, mimetype='application/octet-stream')
            
            response['Content-Disposition'] = 'attachment; filename=%s.zip' % file_name.encode('utf-8')
            
            return response
        else:
            return HttpResponse(simplejson.dumps(False))
    except Exception, e:
        return HttpResponse(simplejson.dumps(False))

def _get_font_name(file_path):
    try:
        #customize path
        f = open(file_path, 'rb')
        #header
        shead = Struct('>IHHHH')
        fhead = f.read(shead.size)
        dhead = shead.unpack_from(fhead, 0)

        #font directory
        stable = Struct('>4sIII')
        ftable = f.read(stable.size * dhead[1])
        for i in range(dhead[1]): 
            #directory records
            dtable = stable.unpack_from(ftable, i * stable.size)
            if dtable[0] == 'name':
                break
        assert dtable[0] == 'name'

        #name table
        f.seek(dtable[2]) #at offset
        fnametable = f.read(dtable[3]) #length
        snamehead = Struct('>HHH') #name table head
        dnamehead = snamehead.unpack_from(fnametable, 0)

        sname = Struct('>HHHHHH')
    except:
        return {}
    NAME_ID = { 1: 'family_name', 4: 'full_name', 6: 'postscript_name' }

    result = {}

    for i in range(dnamehead[1]):
        #name table records
        dname = sname.unpack_from(fnametable, snamehead.size + i * sname.size)
        if dname[3] in NAME_ID:
            _name = unpack_from('%is' % dname[4], fnametable, dnamehead[2] + dname[5])[0]
            try:
                if dname[2] > 0:
                    _name = _name.decode('utf-16-be')
            except:
                pass
            try:
                _name = _name or _name.decode('mbcs')
            except:
                pass
            result.update({ NAME_ID[dname[3]]: _name })
    _compact_full_name = result[NAME_ID[4]].replace(' ', '')
    if NAME_ID[6] not in result or len(_compact_full_name) > len(result[NAME_ID[6]]):
        result.update({NAME_ID[6]: _compact_full_name })
    return result

def _generate_zip_file(file_path, file_name):
    if not os.path.exists(file_path):
        return

    file_name_without_ext = os.path.splitext(file_name)[0]
    md5 = os.path.basename(file_path)
    zip_path = '%s_%s.zip' % (file_path, file_name_without_ext)
    try:
        with ZipFile(zip_path.encode(SYSTEM_ENCODING), 'w') as font_zip:
            font_zip.write(file_path, file_name, ZIP_DEFLATED)
    except Exception,e:
        pass
    return zip_path

def _filter_file_name(filename):
    return re.sub(r'[*?:\\/"><|]','',filename)

def sync_font(request, ext):
    _is_cad = ext == CAD_FONT
    _hash_list = request.POST.get('fontlist','').split(',')
    _sync_result = dict(upload=[])
    _all_fonts = list(Font.objects.filter(sys_font = not _is_cad))
    for _hash in _hash_list:
        if len(_hash) == 0:
            continue
        if _is_cad:
            _base_name, _ext_name = os.path.splitext(_hash.lower())
            _search_result = [item for item in _all_fonts if item.postscript_name.lower() == _base_name and item.file_ext == _ext_name]
        else:
            _search_result = [item for item in _all_fonts if item.file_hash == _hash]
        if len(_search_result) == 0:
            _sync_result['upload'].append(_hash)
        else:
            _all_fonts.remove(_search_result[0])

    #_sync_result['download'] = _is_cad and [_font.postscript_name + _font.file_ext for _font in _all_fonts] or [_font.postscript_name for _font in _all_fonts]

    return HttpResponse(simplejson.dumps(_sync_result))

def list_files(request,type):
    all_uploaded_files = os.listdir(type == CAD_FONT and CAD_UPLOAD_DIRS or SYS_UPLOAD_DIRS)
    for i in range(len(all_uploaded_files)):
        try:
            all_uploaded_files[i].decode(SYSTEM_ENCODING)
        except:
            pass
    return render(request, 'FontList/uploaded_files.html',{'files':all_uploaded_files})

def remove_files(request,type):
    base_dir = type == CAD_FONT and CAD_UPLOAD_DIRS or SYS_UPLOAD_DIRS
    all_uploaded_files = os.listdir(base_dir)
    all_uploaded_files = [filename for filename in all_uploaded_files]
    for file in all_uploaded_files:
        try:
            os.remove(base_dir + file)
        except:
            continue
    return HttpResponse('%d files removed in %s folder' % (len(all_uploaded_files), type))

def make_lower(request):
    current_page_size = max_page_size = 200
    start_index = 0
    while current_page_size == max_page_size:
        error_code, obj_list = baebcs.list_objects('fontcenter',start=start_index, limit=max_page_size)
        if error_code == 0:
            current_page_size = len(obj_list)
            start_index += current_page_size
            for name in obj_list:
                if name == name.lower():
                    continue
                else:
                    baebcs.copy_object('fontcenter',name,'fontcenter',name.lower())
                    baebcs.del_object('fontcenter',name)
        else:
            return HttpResponse('error: %s' % error_code)
    return HttpResponse('%d files' % len(obj_list))

def report_missing_font(request, type):
    if request.method == 'POST':
        keyword = request.POST['keyword']
    if not keyword:
        return HttpResponse(simplejson.dumps('keyword required'))
    try:
        MissingLog.objects.get(font_name= keyword, sys_font= type != CAD_FONT)
        return HttpResponse(simplejson.dumps('record exists'))
    except:
        MissingLog.objects.create(font_name= keyword, sys_font= type != CAD_FONT)
        return HttpResponse(simplejson.dumps('record created'))