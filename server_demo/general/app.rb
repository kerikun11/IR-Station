require 'sinatra'
require 'sinatra/reloader'
require 'active_record'
require 'sinatra/json'

chName = [
	"Light1 ON", "Light1 OFF", "Bright1", "Dark1", "Night1",
	"Light2 ON", "Light2 OFF", "Bright2", "Dark2", "Night2",
	"Light3 ON", "Light3 OFF", "Bright3", "Dark3", "Night3",
	"Light4 ON", "Light4 OFF", "Bright4", "Dark4", "Night4",
	"Light5 ON", "Light5 OFF", "Bright5", "Dark5", "Night5",
]

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get "/recode" do
	sleep(2)
	logger.info "ch = "+params[:ch]
	logger.info "chName = "+params[:chName]
	json "Recode a signal ch"+params[:ch]+" as "+chName[params[:ch].to_i-1]
end

get "/send" do
	sleep(1)
	logger.info "ch = "+params[:ch]
	json "Sent a signal ch"+params[:ch]+": "+chName[params[:ch].to_i-1]
end

get "/clearAllSignals" do
	sleep(2)
	json "Cleared"
end

get "/disconnectWifi" do
	sleep(1)
	json "Disconnected"
end

get"/chName" do
	sleep(1)
	json chName
end

get"/status" do
	sleep(1)
	status = ["Listening..."]
	json status
end

get"/info" do
	sleep(1)
	info = ["Listening...","WiFi-2.4GHz","192.168.11.8","http://esp8266.local"]
	json info
end

