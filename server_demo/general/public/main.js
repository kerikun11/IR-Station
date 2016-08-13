function updateStatus(status){
	$('span#info-status').text(status);
	$('#log').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+' => '+status+'</p>');
}
function loadChName(){
	$.getJSON('/name-list',{},function(data) {
		$('#send').empty();
		$('#manage select[name="ch"]').empty();
		$('#manage select[name="ch"]').append($('<option>').val(-1).text("-select-"));
		for(var i=0;i<data.length;i++){
			var name = data[i];
			$('#send').append($('<button>').val(i+1).text((name=="")?("ch "+(i+1)):name));
			$('#manage select[name="ch"]').append($('<option>').val(i+1).text("ch"+(i+1)+"("+name+")"));
		}
	});
}

/* menu */
$('#menu-signal').click(
	function(){
		$('#advanced').hide();
		$('#manage').toggle();
	}
);
$('#menu-advanced').click(
	function(){
		$('#manage').hide();
		$('#advanced').toggle();
	}
);

/* send */
$(document).on('click','#send button',function(){
	var el = $(this);
	el.addClass("sending");
	$.get('/send',{ch: el.val()}).done(
		function(res){
			//$('#log').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+' => '+res+'</p>');
			el.removeClass("sending");
			updateStatus(res);
		}
	);
});

/* manage signals */
$('#manage select[name="action"]').change(function(){
	var action = $(this).val();
	if(action == "recode"){
		$('#p-name').show();
		$('#p-file').hide();
	}else if(action == "rename"){
		$('#p-name').show();
		$('#p-file').hide();
	}else if(action == "upload"){
		$('#p-name').show();
		$('#p-file').show();
	}else if(action == "download"){
		$('#p-name').hide();
		$('#p-file').hide();
	}else if(action == "clear"){
		$('#p-name').hide();
		$('#p-file').hide();
	}
});
function manage(){
	if($('#manage select[name="ch"]').val() == -1){
		alert("Please select a channel!");
		return;
	}
	var ch = $('#input-ch').val();
	var name = $('#input-name').val();
	var action = $('#manage select[name="action"]').val();
	if(action == "recode"){
		$('#manage').hide();
		$.get('/recode',{
			ch: ch,
			name: name
		}).done(function(res){
			$('#input-name').val("");
			updateStatus(res);
			loadChName();
		});
	}else if(action == "rename"){
		$('#manage').hide();
		$.get('/rename',{
			ch: ch,
			name: name
		}).done(function(res){
			$('#input-name').val("");
			updateStatus(res);
			loadChName();
		});
	}else if(action == "upload"){
		var uploadFile = document.getElementById('input-file');
		var file = uploadFile.files[0];
		if(!file){
			alert("Please select a file");
		}else{
			var reader = new FileReader();
			reader.readAsText(file);
			reader.onload = function(){
				$.get('/upload',{
					ch: ch,
					name: name,
					irJson: reader.result
				}).done(function(res){
					$('#input-name').val("");
					$('#input-file').val("");
					$('#manage').hide();
					updateStatus(res);
					loadChName();
				});
			}
		}
	}else if(action == "download"){
		var a = document.createElement('a');
		var ch = ch
		a.download = ch+".json",
		a.href = "/IR_data/"+ch+".json"
		var evt = document.createEvent('MouseEvent');
		evt.initEvent("click", true, false);
		a.dispatchEvent(evt);
		$('#input-ch').attr('selected',false);
		$('#input-ch').val(ch-0+1);
	}else if(action == "clear"){
		updateStatus("Cleaning...");
		$('#manage').hide();
		$.get('/clear',{
			ch: ch,
		}).done(function(res){
			updateStatus(res);
			loadChName();
		});
	}
}
$('#manage button').click(manage);
$('#manage input').keypress(function(e){
	if(e.which == 13){
		manage();
	}
});

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

