function loadChName(){
	$.getJSON('/chName',{},function(data) {
		console.log(data);
		for(var i=0;i<25;i++){
			$('#send button').eq(i).text(data[i]);
			$('#recode option').eq(i+1).text("ch "+(i+1).toString()+": "+data[i]);
		}
	});
}
function init(){
	loadChName();
	$.getJSON('/info',{},function(data) {
		console.log(data);
		$('span#info-status').text(data[0]);
		$('span#info-ssid').text(data[1]);
		$('span#info-ipaddress').text(data[2]);
		$('span#info-url').text(data[3]);
		$('#main').toggle();
	});
}
function updateStatus(status){
	$('span#info-status').text(status);
}
function recode(){
	if($('#recode select[name="ch"]').val() == -1){
		alert("Please select a channel!");
	}else{
		$('#recode').toggle();
		$.get('/recode',{
			ch: $('#recode select[name="ch"]').val(),
			chName: $('#recode input[name="chName"]').val()
		}).done(function(res){
			$('#recode input[name="chName"]').val("");
			updateStatus(res);
			loadChName();
		});
	}
}
$('#menu-recode').click(
	function(){
		$('#advanced').hide();
		$('#recode').toggle();
	}
);
$('#menu-advanced').click(
	function(){
		$('#recode').hide();
		$('#advanced').toggle();
	}
);
$('#send button').click(
	function(){
		var el = $(this);
		el.addClass("sending");
		$.get('/send',{ch: el.val()}).done(
			function(res){
				$('#log').prepend('<p>'+Date().match(/.+(\d\d:\d\d:\d\d).+/)[1]+' => '+res+'</p>');
				el.removeClass("sending");
				updateStatus(res);
			}
		);
	}
);
$('#recode button').click(recode);
$('#recode input').keypress(function(e){
	if(e.which == 13){
		recode();
	}
});
$('#clearAllSignals').click(
	function(){
		if(confirm('Are you sure to delete all signals?')){
			$.get('/clearAllSignals').done(function(res){
				loadChName();
				updateStatus(res);
				$('#advanced').toggle();
			});
		}
	}
);
$('#disconnectWifi').click(
	function(){
		if(confirm('Are you sure to disconnect this WiFi?')){
			$.get('/disconnectWifi');
			$('#advanced').toggle();
			$('#main').toggle();
		}
	}
);
init();

