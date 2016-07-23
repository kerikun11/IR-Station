function getWifiList(){
	console.log("Getting wifi list")
	$.getJSON(
		'/wifiList',
		{},
		function(data) {
			$('#wifiList').empty();
			$.each(data,function(index,wifi){
				$('#wifiList').append($('<option>').val(wifi).text(wifi));
			});
			$('#info-status').text("Loading successful. Select a mode.")
			$('#form').show();
			$('#ap').show();
		}
	).fail(function(){
		$('#info-status').text('Connection Failed. Please Reload.');
	});
}
function form(){
	if(confirm("Are you sure to confirm?")){
		$('#form').hide();
		$('#ap').hide();
		$('#info-status').text("Connecting... Please wait.");
		$.get('/confirm',{
			ssid: $('#form [name="ssid"]').val(),
			password: $('#form input[name="password"]').val(),
			url: $('#form input[name="url"]').val()
		}).done(function(res){
			var url = $('#form input[name="url"]').val();
			if(res=="false"){
				$('#form').show();
				$('#ap').show();
				$('#info-status').text("Connection failed. Please try again.");
			}else{
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
			$('#form').show();
			$('#ap').show();
			$('#info-status').text("Connection failed. Please try again.");
		});
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

