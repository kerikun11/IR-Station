function long2ip(ip){
	return [ip >>> 0 & 0xFF, ip >>> 8 & 0xFF, ip >>> 16 & 0xFF, ip >>> 24 & 0xFF].join('.')
}
function updateStatus(status){
	$('span#info-status').text(status);
	$('#log-area').prepend($('<p>').text(Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+': '+status));
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
			$('#send td').eq(column_max*(row-1)+(column-1)).html($("<button>").val(id).text(name).addClass("btn btn-default"));
			$('#input-id').append($('<option>').val(id).text("["+row+", "+column+"] "+name));
		});
		// informations
		$('span#info-version').text(data["version"]);
		$('span#info-hostname').text(data["hostname"]);
		$('span#info-ssid').text(data["ssid"]);
		var local_ip = data["local_ip"];
		var subnetmask = data["subnetmask"];
		var gateway = data["gateway"];
		$('span#info-local_ip').text(long2ip(local_ip));
		$('#input-local_ip').val(long2ip(local_ip))
		$('#input-subnetmask').val(long2ip(subnetmask))
		$('#input-gateway').val(long2ip(gateway))
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
	$('#input-time').val((new Date((new Date()).getTime()+9*60*60*1000)).toISOString().substring(0,17)+"00");
  var options = Object.values($('.form-group.list').map((_,el)=>`#${el.id}`));
  var UI = {
    record:  ['#form-position', '#form-name'],
		rename:  ['#form-id','#form-name'],
		move:    ['#form-id','#form-position'],
		upload:  ['#form-position','#form-name','#form-file'],
		download:['#form-id'],
		clear:   ['#form-id'],
		clearAll: [],
		disconnectWifi: [],
		scheduleNew:['#form-id','#form-time'],
    alexadd:    ['#form-alexadd'],
		changeIp:   ['#form-ipaddress'] };

  options.forEach((el,_)=>$(el).hide());
  UI[action].forEach((el,_)=>$(el).show());
	});
function manage(){
	var action = $('#input-action').val();
	var id = $('#input-id').val();
	var name = $('#input-name').val();
	var row = $('#input-row').val()-0;
	var column = $('#input-column').val()-0;
	switch(action){
		case "record":
			if(row>25 || row<0)return $('#form-submit label').text("row is no more than 25");
			if(column>10 || column<0)return $('#form-submit label').text("column is no more than 10");
			if(name == "")return $('#form-submit label').text("Please type a name");
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
			if(id == -1)return $('#form-submit label').text("Select a signal");
			if(name == "")return $('#form-submit label').text("Please type a name");
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
			if(id == -1)return $('#form-submit label').text("Select a signal");
			if(row>25 || row<0)return $('#form-submit label').text("row is no more than 25");
			if(column>10 || column<0)return $('#form-submit label').text("column is no more than 10");
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
			if(row>25 || row<0)return $('#form-submit label').text("row is no more than 25");
			if(column>10 || column<0)return $('#form-submit label').text("column is no more than 10");
			if(name == "")return $('#form-submit label').text("Please type a name");
			var uploadFile = document.getElementById('input-file');
			var file = uploadFile.files[0];
			if(!file)return $('#form-submit label').text("Select a file");
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
			updateStatus("Uploading a new Signal...");
			break;
		case "download":
			if(id == -1)return $('#form-submit label').text("Select a signal");
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
			if(id == -1)return $('#form-submit label').text("Select a signal");
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
			if(id == -1)return $('#form-submit label').text("Select a signal");
			var time = $('#input-time').val();
			if(!time.match(/^20\d\d(.\d\d){4}(...)?$/))return $('#form-submit label').text("Invalid Time");
			var time = Math.floor(new Date($('#input-time').val()).getTime()/1000);
			if(time < Math.floor(new Date().getTime()/1000))return $('#form-submit label').text("No Past Time!");
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
			if(!confirm('Are you sure to disconnect this WiFi?'))return;
			$.post('/wifi/disconnect');
			$('#main').hide();
			break;
		case "change-ip":
			var local_ip = $('#input-local_ip').val();
			var subnetmask = $('#input-subnetmask').val();
			var gateway = $('#input-gateway').val();
			if(!local_ip.match(/^\d{1,3}(\.\d{1,3}){3}$/))return $('#form-submit label').text("Invalid IP address");
			if(!subnetmask.match(/^\d{1,3}(\.\d{1,3}){3}$/))return $('#form-submit label').text("Invalid Subnet Mask");
			if(!gateway.match(/^\d{1,3}(\.\d{1,3}){3}$/))return $('#form-submit label').text("Invalid Gateway IP");
			if(!confirm('Are you sure to change ip address?')) return;
			$('#main').hide();
			$.post('/wifi/change-ip',{
				local_ip: $('#input-local_ip').val(),
				subnetmask: $('#input-subnetmask').val(),
				gateway: $('#input-gateway').val()
			}).done(function(res){
				updateStatus(res);
				$('#main').show();
			}).fail(function(){
				location.href = "http://"+$('#input-local_ip').val();
			})
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
