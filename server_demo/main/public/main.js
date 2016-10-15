var station;
function updateStatus(status){
	$('span#info-status').text(status);
	$('#log-area').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+': '+status+'</p>');
}
function load(){
	$.getJSON('/info',{},function(data) {
		station = data;
		// send buttons settings id list
		$('#send tbody').empty();
		$('#input-id').empty();
		$('#input-id').append($('<option>').val(-1).text("-select-"));
		var signals = data["signals"];
		var row_max = 0;
		var column_max = 0;
		data["signals"].forEach(function(signal){
			var row = signal["row"];
			var column = signal["column"];
			if(row>row_max)row_max=row;
			if(column>column_max)column_max=column;
		});
		for(var i=1; i<=row_max; i++){
			$("#send>table>tbody").append($('<tr>').val(i));
			for(var j=1; j<=column_max; j++){
				$("#send>table>tbody>tr").eq(i-1).append($('<td>').val(j).text("["+i+", "+j+"]"));
			}
		}
		data["signals"].forEach(function(signal){
			var id = signal["id"];
			var name = signal["name"];
			var row = signal["row"]-0;
			var column = signal["column"]-0;
			$('#send td').eq(column_max*(row-1) + (column-1)).html($("<button>").val(id).text(name).addClass("btn btn-default"));
			$('#input-id').append($('<option>').val(id).text("["+row+", "+column+"] "+name));
		});
		// informations
		$('span#info-version').text(data["version"]);
		$('span#info-hostname').text(data["hostname"]);
		$('span#info-ssid').text(data["ssid"]);
		var ip = data["local_ip"];
		$('span#info-local_ip').text(""+((ip>>0)&0xFF)+"."+((ip>>8)&0xFF)+"."+((ip>>16)&0xFF)+"."+((ip>>24)&0xFF));
		// schedule
		var schedule = data["schedules"];
		$('#schedule-list').empty();
		schedule.forEach(function(item){
			var name="";
			data["signals"].forEach(function(signal){
				if(signal["id"] == item["id"]){
					name = signal["name"];
				}
			});
			var time = new Date(Number(item["time"]*1000));
			$('#schedule-list').append(
				$('<li>')
				.append($('<span>').text(name+" at "+time.toLocaleString()+" "))
				.append($('<button>').text('delete').addClass("schedule-delete"))
				.val(item["schedule_id"])
			);
		});
	}).done(function(){
		$('#main').show();
	});
}

/* manage signals */
$('#manage select[name="action"]').change(function(){
	var action = $(this).val();
	$('#form-submit label').text("")
	switch(action){
		case "record":
			$('#form-id').hide();
			$('#form-position').show();
			$('#form-name').show();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "rename":
			$('#form-id').show();
			$('#form-position').hide();
			$('#form-name').show();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "move":
			$('#form-id').show();
			$('#form-position').show();
			$('#form-name').hide();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "upload":
			$('#form-id').hide();
			$('#form-position').show();
			$('#form-name').show();
			$('#form-file').show();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "download":
		case "clear":
			$('#form-id').show();
			$('#form-position').hide();
			$('#form-name').hide();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "clear-all":
		case "disconnect-wifi":
			$('#form-id').hide();
			$('#form-position').hide();
			$('#form-name').hide();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').hide();
			break;
		case "schedule-new":
			$('#form-id').show();
			$('#form-position').hide();
			$('#form-name').hide();
			$('#form-file').hide();
			$('#form-time').show();
			$('#form-ipaddress').hide();
			break;
		case "change-ip":
			$('#form-id').hide();
			$('#form-position').hide();
			$('#form-name').hide();
			$('#form-file').hide();
			$('#form-time').hide();
			$('#form-ipaddress').show();
			break;
	}
});
function manage(){
	var action = $('#input-action').val();
	var id = $('#input-id').val();
	var name = $('#input-name').val();
	var row = $('#input-row').val()-0;
	var column = $('#input-column').val()-0;
	switch(action){
		case "record":
			if(row<=0){
				$('#form-submit label').text("row is positive number")
				return;
			}
			if(column > 6 || column<=0){
				$('#form-submit label').text("column is no more than 5")
				return;
			}
			if(name == ""){
				$('#form-submit label').text("Please type a name")
				return;
			}
			$.post('/signals/record',{
				name: name,
				display: true,
				row: row,
				column: column
			}).done(function(res){
				$('#input-name').val("");
				updateStatus(res);
				load();
			});
			updateStatus("Recording a new Signal...");
			break;
		case "rename":
			if(id == -1){
				$('#form-submit label').text("Select a signal")
				return;
			}
			if(row<0){
				$('#form-submit label').text("row is positive number")
				return;
			}
			if(column > 6 || column<0){
				$('#form-submit label').text("column is no more than 5")
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
			break;
		case "move":
			if(id == -1){
				$('#form-submit label').text("Select a signal")
				return;
			}
			if(row<0){
				$('#form-submit label').text("row is positive number")
				return;
			}
			if(column > 6 || column<0){
				$('#form-submit label').text("column is no more than 6")
				return;
			}
			$.post('/signals/move',{
				id: id,
				row: row,
				column: column
			}).done(function(res){
				updateStatus(res);
				load();
			});
			break;
		case "upload":
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
						row: row,
						column: column,
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
			break;
		case "download":
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
			break;
		case "clear":
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
			break;
		case "clear-all":
			if(confirm('Are you sure to delete all signals?')){
				updateStatus("Cleaning...");
				$.post('/signals/clear-all').done(function(res){
					load();
					updateStatus(res);
				});
			}
			break;
		case "schedule-new":
			if(id == -1){
				$('#form-submit label').text("Select a signal")
				return;
			}
			var time = $('#input-time').val();
			if(!time.match(/^20\d\d(.\d\d){4}(...)?$/)){
				$('#form-submit label').text("Invalid Time");
				return;
			}
			var time = Math.floor(new Date($('#input-time').val()).getTime()/1000)-9*60*60;
			if(time < Math.floor(new Date().getTime()/1000)){
				$('#form-submit label').text("No Past Time");
				return;
			}
			$.post('/schedule/new',{
				id: id,
				time: time
			}).done(function(res){
				updateStatus(res);
				load();
				$('#input-time').val("");
			});
			break;
		case "disconnect-wifi":
			if(confirm('Are you sure to disconnect this WiFi?')){
				$.post('/wifi/disconnect');
				$('#main').hide();
			}
			break;
		case "change-ip":
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
			break;
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
updateStatus("Loading Successful :)");

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

/* position */
$(document).on('click','#send td',function(){
	var row = $(this).parent().val();
	var column = $(this).val();
	$('#input-row').val(row);
	$('#input-column').val(column);
});

/* schedule delete */
$(document).on('click','button.schedule-delete',function(){
	$.get('/schedule/delete',{
		schedule_id: $(this).parent().val()
	}).done(function(){
		load();
	});
});
