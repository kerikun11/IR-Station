require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

wifiList = [
	"WiFI-ssid",
	"ABC",
	"hello",
	"ESP8266",
	"ESP-WROOM-02",
]

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get '/wifi/list' do
	sleep(1)
	json wifiList
end

get"/status" do
	status = ["Listening..."]
	json status
end

get '/mode/station' do
	sleep(1)
	"OK"
end

get '/wifi/confirm' do
	"false"
	"192.168.11.6"
end

get '/mode/accesspoint' do
	sleep(1)
	"Access Point Mode"
end

