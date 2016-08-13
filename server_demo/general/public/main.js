function updateStatus(status){
	$('span#info-status').text(status);
	$('#log-area').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+': '+status+'</p>');
}
function loadChName(){
	$.getJSON('/name-list',{},function(data) {
		$('#send').empty();
		$('#manage select[name="ch"]').empty();
		$('#manage select[name="ch"]').append($('<option>').val(-1).text("-select-"));
		for(var i=0;i<data.length;i++){
			var name = data[i];
			$('#send').append($('<button>').val(i+1).text((name=="")?("ch "+(i+1)):name).addClass("btn btn-default"));
			$('#manage select[name="ch"]').append($('<option>').val(i+1).text((i+1)+" ch ("+name+")"));
		}
	});
}

/* send */
$(document).on('click','#send button',function(){
	var el = $(this);
	el.addClass("sending");
	$.get('/send',{ch: el.val()}).done(
		function(res){
			el.removeClass("sending");
			updateStatus(res);
		}
	);
});

/* manage signals */
$('#manage select[name="action"]').change(function(){
	var action = $(this).val();
	if(action == "recode"){
		$('#form-ch').show();
		$('#form-name').show();
		$('#form-file').hide();
	}else if(action == "rename"){
		$('#form-ch').show();
		$('#form-name').show();
		$('#form-file').hide();
	}else if(action == "upload"){
		$('#form-ch').show();
		$('#form-name').show();
		$('#form-file').show();
	}else if(action == "download"){
		$('#form-ch').show();
		$('#form-name').hide();
		$('#form-file').hide();
	}else if(action == "clear"){
		$('#form-ch').show();
		$('#form-name').hide();
		$('#form-file').hide();
	}else if(action == "clear-all"){
		$('#form-ch').hide();
		$('#form-name').hide();
		$('#form-file').hide();
	}else if(action == "disconnect-wifi"){
		$('#form-ch').hide();
		$('#form-name').hide();
		$('#form-file').hide();
	}
});
function manage(){
	var ch = $('#input-ch').val();
	var name = $('#input-name').val();
	var action = $('#input-action').val();
	if(action == "recode"){
		if($('#manage select[name="ch"]').val() == -1){
			$('#form-submit label').text("Select a channel")
			return;
		}
		$.get('/recode',{
			ch: ch,
			name: name
		}).done(function(res){
			updateStatus(res);
			loadChName();
		});
		$('#input-name').val("");
	}else if(action == "rename"){
		if($('#manage select[name="ch"]').val() == -1){
			$('#form-submit label').text("Select a channel")
			return;
		}
		$.get('/rename',{
			ch: ch,
			name: name
		}).done(function(res){
			$('#input-name').val("");
			updateStatus(res);
			loadChName();
		});
	}else if(action == "upload"){
		if($('#manage select[name="ch"]').val() == -1){
			$('#form-submit label').text("Select a channel")
			return;
		}
		var uploadFile = document.getElementById('input-file');
		var file = uploadFile.files[0];
		if(!file){
			$('#form-submit label').text("Select a file")
			return;
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
					updateStatus(res);
					loadChName();
				});
			}
		}
	}else if(action == "download"){
		if($('#manage select[name="ch"]').val() == -1){
			$('#form-submit label').text("Select a channel")
			return;
		}
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
		if($('#manage select[name="ch"]').val() == -1){
			$('#form-submit label').text("Select a channel")
			return;
		}
		updateStatus("Cleaning...");
		$.get('/clear',{
			ch: ch,
		}).done(function(res){
			updateStatus(res);
			loadChName();
		});
	}else if(action == "clear-all"){
		updateStatus("Cleaning...");
		if(confirm('Are you sure to delete all signals?')){
			updateStatus("Cleaning...");
			$.get('/clear-all').done(function(res){
				loadChName();
				updateStatus(res);
			});
		}
	}else if(action == "disconnect-wifi"){
		if(confirm('Are you sure to disconnect this WiFi?')){
			$.get('/disconnect-wifi');
			$('#main').hide();
		}
	}
	$('#form-submit label').text("")
}
$('#manage button').click(manage);
$('#manage input').keypress(function(e){
	if(e.which == 13){
		manage();
	}
});

/* init */
function init(){
	loadChName();
	$.getJSON('/info',{},function(data) {
		$('span#info-status').text(data[0]);
		$('span#info-ssid').text(data[1]);
		$('span#info-ipaddress').text(data[2]);
		$('span#info-url').text(data[3]);
		$('#main').show();
	});
}
init();

