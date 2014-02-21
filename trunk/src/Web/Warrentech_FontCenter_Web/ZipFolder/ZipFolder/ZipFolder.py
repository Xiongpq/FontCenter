import os
import sys
from zipfile import ZipFile, ZIP_DEFLATED, is_zipfile
import glob

def zip_folder(folder_path, zip_size):
    if not os.path.exists(folder_path):
        print '[Error] Path not found'
        return
    all_file_names = os.listdir(folder_path)

    file_info = []
    for file_name in all_file_names:
        file_path = folder_path + file_name
        if os.path.isdir(file_path):
            continue
        file_info.append({'path':file_path,'size':os.path.getsize(file_path)})
    file_info.sort(key=lambda x:x['size'])
    index_count = 0
    size_count = 0
    if not os.path.exists('ziped'):
        os.makedirs('ziped')
    else:
        previous_zip_files = glob.glob(folder_path + '\ziped\*.zip')
        for f in previous_zip_files:
            os.remove(f)
    while index_count < len(file_info):
        file = file_info[index_count]
        if size_count == 0 and 'font_zip' not in locals():
            single_zip_capacity = 0
            font_zip = ZipFile('%s\ziped\%d.zip' % (folder_path,index_count), 'w')
        if (single_zip_capacity > 0 and (size_count + file['size']) * 0.6 > zip_size) or single_zip_capacity == 20 :
            size_count = 0
            font_zip.close()
            del font_zip
            continue
        else:
            single_zip_capacity+=1
            index_count+=1
            __file_path_to_ziped = file['path']
            __filename_to_stored = os.path.basename(__file_path_to_ziped)
            font_zip.write(__file_path_to_ziped, __filename_to_stored, ZIP_DEFLATED)
            size_count += file['size']
                    
def zip_folder_by_name(folder_path):
    if not os.path.exists(folder_path):
        print '[Error] Path not found'
        return
    all_file_names = os.listdir(folder_path)

    file_info = []
    for file_name in all_file_names:
        file_path = folder_path + file_name
        if os.path.isdir(file_path):
            continue
        file_info.append({'path':file_path})
    if not os.path.exists('ziped'):
        os.makedirs('ziped')
    else:
        previous_zip_files = glob.glob(folder_path + '\ziped\*.zip')
        for f in previous_zip_files:
            os.remove(f)
    for file in file_info:
        __file_path_to_ziped = file['path']
        __filename_to_stored = os.path.basename(__file_path_to_ziped)
        with ZipFile('%s\ziped\%s.zip' % (folder_path,__filename_to_stored), 'w') as font_zip:
            font_zip.write(__file_path_to_ziped, __filename_to_stored, ZIP_DEFLATED)

if len(sys.argv) > 1:
    #zip_size = (len(sys.argv) > 2 and sys.argv[2] or 2) * 1024 * 1024
    #zip_folder(sys.argv[1],zip_size)
    zip_folder_by_name(sys.argv[1])
else:
    print '[Info] Please specify a valid path'
