function getWifiList(){
	$.getJSON('/wifi/list').done(function(data) {
		$('#wifi-list').empty();
		$.each(data,function(index,wifi){
			$('#wifi-list').append($('<option>').val(wifi).text(wifi));
		});
		$('#wifi-list').append($('<option>').val("stealth-ssid").text("- use a stealth SSID -"));
		$('#info-status').text("Loading successful. Select a mode.")
		$('#form').show();
		$('#ap').show();
	}).fail(function(){
		$('#info-status').text('Connection Failed. Please Reload.');
	});
}
function setStation(){
	$('#form').hide();
	$('#ap').hide();
	$('#info-status').text("Connecting... Please wait...");
	$.get('/mode/station',{
		hostname: $('#form input[name="hostname"]').val(),
		is_stealth_ssid: ($('#form [name="ssid"]').val()=="stealth-ssid"),
		ssid: ($('#form [name="ssid"]').val()=="stealth-ssid")?$('#form [name="stealth-ssid"]').val():$('#form [name="ssid"]').val(),
		password: $('#form input[name="password"]').val()
	}).fail(function(){
		$('#info-status').text('Connection Failed. Please Reload.');
	});
	var cnt = 0;
	timerID = setInterval(function(){
		cnt++;
		if(cnt > 20){
			$('#form').show();
			$('#ap').show();
			$('#info-status').text("Connection failed. Please try again.");
			clearInterval(timerID);
			timerID = null;
		}
		$.get('/wifi/confirm').done(function(res){
			var hostname = $('#form input[name="hostname"]').val();
			if(res!="false"){
				clearInterval(timerID);
				timerID = null;
				$('#info-status').html(
					'Connection Successful.<br/>URL: <a href="http://'+res+'/" target="_blank">http://'+res+'/</a>'
				);
			}
		}).fail(function(){
			$('#info-status').text('Connection Failed. Please Reload.');
		});
	}, 1000);
}
function setAP(){
	if(confirm("Can I setup as Access Point Mode?")){
		$('#form').toggle();
		$('#ap').toggle();
		$('#info-status').text("Connecting... Please wait.");
		$.get('/mode/accesspoint',{
			hostname: $('#ap input[name="hostname"]').val()
		}).done(function(res){
			$('#info-status').text(res);
		}).fail(function(){
			$('#info-status').text('Connection Failed. Please Reload.');
		});
	}
}

$('#form button').click(setStation);
$('#form input').keypress(function(e){
	if(e.which == 13){
		setStation();
	}
});

$('#ap button').click(setAP);
$('#ap input').keypress(function(e){
	if(e.which == 13){
		setAP();
	}
});

$('#wifi-list').change(function(){
	if($('#wifi-list option:selected').val()=="stealth-ssid"){
		$('#stealth-ssid-form').show();
	}else{
		$('#stealth-ssid-form').hide();
	}
});

getWifiList();

