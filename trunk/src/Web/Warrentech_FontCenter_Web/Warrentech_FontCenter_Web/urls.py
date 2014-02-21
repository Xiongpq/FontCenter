from django.conf.urls import patterns, include
from django.contrib import admin
from Warrentech_FontCenter_Web.views.FontListViews import *
from django.views.generic import RedirectView

admin.autodiscover()

urlpatterns = patterns('',

    (r'^$', font_list, {'keyword':''}),
    (r'^admin/Warrentech_FontCenter_Web/font/add/', font_list, {'keyword' : ''}),
    (r'^admin/', include(admin.site.urls)),
    (r'^upload_(?P<type>\w*)_font$', upload_font),
    #(r'^list_(?P<type>.*)_files', list_files),
    #(r'^remove_(?P<type>.*)_files', remove_files),
    #(r'^download_(?P<type>\w{3})_font/(?P<keyword>.*)', get_font),
    (r'^sync_(?P<ext>\w{3})_font', sync_font),
    #(r'^download/(?P<type>\w{3})/(?P<md5>[0-9a-f]{32})/(?P<file_name>.+)', download),
    #(r'^make_lower', make_lower),
    (r'^report_missing_(?P<type>\w{3})_font$', report_missing_font),
    (r'^favicon\.ico$', RedirectView.as_view(url='/static/css/common/images/favicon.ico')),
    )
