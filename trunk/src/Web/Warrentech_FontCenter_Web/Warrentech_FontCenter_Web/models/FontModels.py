from django.db import models

# Create your models here.
class Font(models.Model):
    family_name = models.CharField(max_length=256, null=True)
    full_name = models.CharField(max_length=256, null=True)
    postscript_name = models.CharField(max_length=256)
    file_ext = models.CharField(max_length=256)
    file_hash = models.CharField(max_length=32)
    sys_font = models.BooleanField()

    def __unicode__(self):
        if self.sys_font:
            return self.full_name or self.postscript_name
        else:
            return self.postscript_name + self.file_ext
    class Meta:
        ordering = ['postscript_name']
        db_table = 'FontCenter_font'
        app_label = 'Warrentech_FontCenter_Web'
        
