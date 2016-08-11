function updateStatus(status){
	$('span#info-status').text(status);
	$('#log').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+' => '+status+'</p>');
}
function loadChName(){
	$.getJSON('/name-list',{},function(data) {
		for(var i=0;i<25;i++){
			$('#send button').eq(i).text(data[i]);
			$('#signal option').eq(i+1).text("ch "+(i+1).toString()+": "+data[i]);
		}
	});
}

/* menu */
$('#menu-signal').click(
	function(){
		$('#advanced').hide();
		$('#signal').toggle();
	}
);
$('#menu-advanced').click(
	function(){
		$('#signal').hide();
		$('#advanced').toggle();
	}
);

/* send */
$('#send button').click(
	function(){
		var el = $(this);
		el.addClass("sending");
		$.get('/send',{ch: el.val()}).done(
			function(res){
				//$('#log').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+' => '+res+'</p>');
				el.removeClass("sending");
				updateStatus(res);
			}
		);
	}
);

/* recode */
$('#signal-recode').click(function(){
	$('#recode').toggle();
	$('#rename').hide();
	$('#upload').hide();
});
function recode(){
	if($('#signal select[name="ch"]').val() == -1){
		alert("Please select a channel!");
	}else{
		$('#signal').hide();
		$.get('/recode',{
			ch: $('#signal select[name="ch"]').val(),
			name: $('#recode input[name="name"]').val()
		}).done(function(res){
			$('#recode input[name="name"]').val("");
			$('#recode').hide();
			updateStatus(res);
			loadChName();
		});
	}
}
$('#recode button').click(recode);
$('#recode input').keypress(function(e){
	if(e.which == 13){
		recode();
	}
});

/* rename */
$('#signal-rename').click(function(){
	$('#recode').hide();
	$('#rename').toggle();
	$('#upload').hide();
});
function rename(){
	if($('#signal select[name="ch"]').val() == -1){
		alert("Please select a channel!");
	}else{
		$('#signal').hide();
		$.get('/rename',{
			ch: $('#signal select[name="ch"]').val(),
			name: $('#rename input[name="name"]').val()
		}).done(function(res){
			$('#rename input[name="name"]').val("");
			$('#rename').hide();
			updateStatus(res);
			loadChName();
		});
	}
}
$('#rename button').click(rename);
$('#rename input').keypress(function(e){
	if(e.which == 13){
		rename();
	}
});

/* upload */
$('#signal-upload').click(function(){
	$('#recode').hide();
	$('#rename').hide();
	$('#upload').toggle();
});
function upload(){
	if($('#signal select[name="ch"]').val() == -1){
		alert("Please select a channel!");
	}else{
		var uploadFile = document.getElementById('upload-file');
		var file = uploadFile.files[0];
		if(!file){
			alert("Please select a file");
		}else{
			var reader = new FileReader();
			reader.readAsText(file);
			reader.onload = function(){
				$.get('/upload',{
					ch: $('#signal select[name="ch"]').val(),
					name: $('#upload input[name="name"]').val(),
					irJson: reader.result
				}).done(function(res){
					$('#signal').hide();
					$('#upload').hide();
					updateStatus(res);
					loadChName();
				});
			}
		}
	}
}
$('#upload button').click(upload);
$('#upload input').keypress(function(e){
	if(e.which == 13){
		upload();
	}
});

/* clear*/
$('#signal-clear').click(
	function(){
		if($('#signal select[name="ch"]').val() == -1){
			alert("Please select a channel!");
		}else{
			if(confirm('Are you sure to delete this signal?')){
				updateStatus("Cleaning...");
				$('#signal').hide();
				$.get('/clear',{
					ch: $('#signal select[name="ch"]').val(),
				}).done(function(res){
					updateStatus(res);
					loadChName();
				});
			}
		}
	}
);

/* advanced */
$('#disconnect-wifi').click(
	function(){
		if(confirm('Are you sure to disconnect this WiFi?')){
			$.get('/disconnect-wifi');
			$('#advanced').toggle();
			$('#main').toggle();
		}
	}
);
/* clear all */
$('#clear-all').click(
	function(){
		if(confirm('Are you sure to delete all signals?')){
			updateStatus("Cleaning...");
			$('#advanced').hide();
			$.get('/clear-all').done(function(res){
				loadChName();
				updateStatus(res);
			});
		}
	}
);

/* init */
function init(){
	loadChName();
	$.getJSON('/info',{},function(data) {
		$('span#info-status').text(data[0]);
		$('span#info-ssid').text(data[1]);
		$('span#info-ipaddress').text(data[2]);
		$('span#info-url').text(data[3]);
		$('#main').toggle();
	});
}
init();

