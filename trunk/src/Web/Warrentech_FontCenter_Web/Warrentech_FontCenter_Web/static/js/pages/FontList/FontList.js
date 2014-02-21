
function BindFontUploader() {
	$('#fileFont').uploadify({
		swf: UrlResource.UploadifySwf,
		uploader: '/upload_any_font',
		auto: true,
		formData: {},
		buttonText: '添加',
		cancelImage: UrlResource.UploadifyCancelImage,
		multi: true,
		fileObjName: 'file',
		queueSizeLimit: 9999,
		requeueErrors: true,
		removeCompleted: true,
		hint: '',
		onQueueComplete: function () {
		},
		onError: function (event, ID, fileObj, errorObj) {

		},
		onUploadSuccess: function (file, data, response) {

		},
		onDialogOpen: function () {

		},
		onDialogClose: function () {
			this.queueData.filesSelected && this.setUploadURL(
				String.format('/upload_{0}_font', confirm('本次上传的是系统字体吗？') ? 'sys' : 'cad'));
		},
		onUploadStart: function () {

		}

	});

}

$(document).ready(function () {
	BindFontUploader();
	$('#fileFont-button').after($('#btnManageFonts'))
});