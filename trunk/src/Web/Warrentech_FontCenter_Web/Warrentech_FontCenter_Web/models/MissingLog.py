from django.db import models

# Create your models here.
class MissingLog(models.Model):
    font_name = models.CharField(max_length=256)
    sys_font = models.BooleanField()

    def __unicode__(self):
            return '[%s] %s' % (self.sys_font and 'sys' or 'cad', self.font_name)
    class Meta:
        ordering = ['sys_font']
        db_table = 'FontCenter_missing_log'
        app_label = 'Warrentech_FontCenter_Web'
        
