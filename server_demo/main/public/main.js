var station;
function updateStatus(status){
	$('span#info-status').text(status);
	$('#log-area').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+': '+status+'</p>');
}
function load(){
	$.getJSON('/info',{},function(data) {
		station = data;
		$('#send').empty();
		$('#input-id').empty();
		$('#input-id').append($('<option>').val(-1).text("-select-"));
		var signals = data["signals"];
		data["signals"].forEach(function(signal){
			var id = signal["id"];
			var name = signal["name"];
			$('#send').append($('<button>').val(id).text((name=="")?("ch"+(id)):name).addClass("btn btn-default"));
			$('#input-id').append($('<option>').val(id).text(name));
		});
		$('span#info-version').text(data["version"]);
		$('span#info-hostname').text(data["hostname"]);
		$('span#info-ssid').text(data["ssid"]);
		var ip = data["local_ip"];
		$('span#info-local_ip').text(""+((ip>>0)&0xFF)+"."+((ip>>8)&0xFF)+"."+((ip>>16)&0xFF)+"."+((ip>>24)&0xFF));
	}).done(function(){
		$('#main').show();
	});
}

/* send */
$(document).on('click','#send button',function(){
	var el = $(this);
	el.addClass("sending");
	$.post('/signals/send',{
		id: el.val()
	}).done(function(res){
		el.removeClass("sending");
		updateStatus(res);
	});
});

/* manage signals */
$('#manage select[name="action"]').change(function(){
	var action = $(this).val();
	$('#form-submit label').text("")
	if(action == "record"){
		$('#form-id').hide();
		$('#form-name').show();
		$('#form-file').hide();
		$('#form-number').hide();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "rename"){
		$('#form-id').show();
		$('#form-name').show();
		$('#form-file').hide();
		$('#form-number').hide();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "upload"){
		$('#form-id').hide();
		$('#form-name').show();
		$('#form-file').show();
		$('#form-number').hide();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "download" || action == "clear"){
		$('#form-id').show();
		$('#form-name').hide();
		$('#form-file').hide();
		$('#form-number').hide();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "number"){
		$('#form-id').hide();
		$('#form-name').hide();
		$('#form-file').hide();
		$('#form-number').show();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "clear-all" || action == "disconnect-wifi"){
		$('#form-id').hide();
		$('#form-name').hide();
		$('#form-file').hide();
		$('#form-number').hide();
		$('#form-local_ip').hide();
		$('#form-subnetmask').hide();
		$('#form-gateway').hide();
	}else if(action == "change-ip"){
		$('#form-id').hide();
		$('#form-name').hide();
		$('#form-file').hide();
		$('#form-number').hide();
		$('#form-local_ip').show();
		$('#form-subnetmask').show();
		$('#form-gateway').show();
	}
});
function manage(){
	var id = $('#input-id').val();
	var name = $('#input-name').val();
	var number = $('#input-number').val();
	var action = $('#input-action').val();
	if(action == "record"){
		if(name == ""){
			$('#form-submit label').text("Please type a name")
			return;
		}
		$.post('/signals/record',{
			name: name,
			display: true,
			row: 0,
			column: 0
		}).done(function(res){
			$('#input-name').val("");
			updateStatus(res);
			load();
		});
		updateStatus("Recording a new Signal...");
	}else if(action == "rename"){
		if(id == -1){
			$('#form-submit label').text("Select a signal")
			return;
		}
		if(name == ""){
			$('#form-submit label').text("Please type a name")
			return;
		}
		$.post('/signals/rename',{
			id: id,
			name: name
		}).done(function(res){
			$('#input-name').val("");
			updateStatus(res);
			load();
		});
	}else if(action == "upload"){
		if(name == ""){
			$('#form-submit label').text("Please type a name")
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
				$.post('/signals/upload',{
					name: name,
					display: true,
					row: 0,
					column: 0,
					irJson: reader.result
				}).done(function(res){
					$('#input-name').val("");
					$('#input-file').val("");
					updateStatus(res);
					load();
				});
			}
		}
		updateStatus("Uploading a new Signal...");
	}else if(action == "download"){
		if(id == -1){
			$('#form-submit label').text("Select a signal")
			return;
		}
		var a = document.createElement('a');
		name = station["signals"].filter(function(value){
			return (value["id"]==id);
		})[0]["name"];
		a.download = name+".json",
		a.href = "/signals/"+id+".json"
		var evt = document.createEvent('MouseEvent');
		evt.initEvent("click", true, false);
		a.dispatchEvent(evt);
		$('#input-id').attr('selected',false);
	}else if(action == "clear"){
		if(id == -1){
			$('#form-submit label').text("Select a signal")
			return;
		}
		updateStatus("Cleaning...");
		$.post('/signals/clear',{
			id: id,
		}).done(function(res){
			updateStatus(res);
			load();
		});
	}else if(action == "clear-all"){
		if(confirm('Are you sure to delete all signals?')){
			updateStatus("Cleaning...");
			$.post('/signals/clear-all').done(function(res){
				load();
				updateStatus(res);
			});
		}
	}else if(action == "disconnect-wifi"){
		if(confirm('Are you sure to disconnect this WiFi?')){
			$.post('/wifi/disconnect');
			$('#main').hide();
		}
	}else if(action == "change-ip"){
		var local_ip = $('#input-local_ip').val();
		var subnetmask = $('#input-subnetmask').val();
		var gateway = $('#input-gateway').val();
		if(!local_ip.match(/^\d{1,3}(\.\d{1,3}){3}$/)){
			$('#form-submit label').text("Invalid IP address");
			return;
		}
		if(!subnetmask.match(/^\d{1,3}(\.\d{1,3}){3}$/)){
			$('#form-submit label').text("Invalid Subnet Mask");
			return;
		}
		if(!gateway.match(/^\d{1,3}(\.\d{1,3}){3}$/)){
			$('#form-submit label').text("Invalid Gateway IP");
			return;
		}
		if(confirm('Are you sure to change ip address?')){
			$('#main').hide();
			$.post('/wifi/change-ip',{
				local_ip: $('#input-local_ip').val(),
				subnetmask: $('#input-subnetmask').val(),
				gateway: $('#input-gateway').val()
			}).done(function(res){
				$('#input-local_ip').val("");
				$('#input-subnetmask').val("");
				$('#input-gateway').val("");
				updateStatus(res);
				$('#main').show();
			}).fail(function(){
				location.href = "http://"+$('#input-local_ip').val();
			})
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
load();
updateStatus("Welcome!");

