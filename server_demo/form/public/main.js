function getWifiList(){
	console.log("Getting wifi list")
	$.ajax({
		type:"GET",
		url:"/wifiList",
		dataType:"json",
		cache:false,
		timeout:10000
	}).done(function(data) {
		$('#wifiList').empty();
		$.each(data,function(index,wifi){
			$('#wifiList').append($('<option>').val(wifi).text(wifi));
		});
		$('#info-status').text("Loading successful. Select a mode.")
		$('#form').show();
		$('#ap').show();
	}).fail(function(){
		$('#info-status').text('Connection Failed. Please Reload.');
	});
}
function form(){
	if(confirm("Are you sure to confirm?")){
		$('#form').hide();
		$('#ap').hide();
		$('#info-status').text("Connecting... Please wait...");
		$.get('/confirm',{
			ssid: $('#form [name="ssid"]').val(),
			password: $('#form input[name="password"]').val(),
			url: $('#form input[name="url"]').val()
		}).fail(function(){
			$('#info-status').text('Connection Failed. Please Reload.');
		});
		var cnt = 0;
		timerID = setInterval(function(){
			cnt++;
			if(cnt > 5){
				$('#form').show();
				$('#ap').show();
				$('#info-status').text("Connection failed. Please try again.");
				clearInterval(timerID);
				timerID = null;
			}
			$.get('/isConnected').done(function(res){
				var url = $('#form input[name="url"]').val();
				if(res!="false"){
					clearInterval(timerID);
					timerID = null;
					$('#info-status').html(
						'Connection Successful.'
						+'<br/>'+
							'Please make a note of these URL.'
						+'<br/>'+
							'Screenshot is also good!'
						+'<br/>'+
							'For Apple device: <a href="http://'+url+'.local/">http://'+url+'.local/</a>'
						+'<br/>'+
							'For all device: <a href="http://'+res+'/">http://'+res+'/</a>'
					);
				}
			}).fail(function(){
				$('#info-status').text('Connection Failed. Please Reload.');
			});
		}, 2000);
	}
}
function setAP(){
	if(confirm("Can I setup as Access Point Mode?")){
		$('#form').toggle();
		$('#ap').toggle();
		$('#info-status').text("Connecting... Please wait.");
		$.get('/accessPointMode',{
			url: $('#ap input[name="url"]').val()
		}).done(function(res){
			$('#info-status').text(res);
		}).fail(function(){
			$('#info-status').text('Connection Failed. Please Reload.');
		});
	}
}

$('#form button').click(form);
$('#form input').keypress(function(e){
	if(e.which == 13){
		form();
	}
});

$('#ap button').click(setAP);
$('#ap input').keypress(function(e){
	if(e.which == 13){
		setAP();
	}
});

getWifiList();

