#-*- coding:utf-8 -*-

import os
import sys
 
os.environ['DJANGO_SETTINGS_MODULE'] = 'Warrentech_FontCenter_Web.settings'
 
path = os.path.dirname(os.path.abspath(__file__)) + '/Warrentech_FontCenter_Web'
if path not in sys.path:
    sys.path.insert(1, path)
 
from django.core.handlers.wsgi import WSGIHandler
from bae.core.wsgi import WSGIApplication
 
application = WSGIApplication(WSGIHandler())

