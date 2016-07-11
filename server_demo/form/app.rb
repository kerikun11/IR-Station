require 'sinatra'
require 'sinatra/reloader'
require 'active_record'
require 'sinatra/json'

wifiList = [
	"WiFI-ssid",
	"ABC",
	"hello",
	"ESP8266",
	"ESP-WROOM-02",
]

get '/' do
	send_file File.join(settings.public_folder,'index.html')
end

get '/wifiList' do
	sleep(1)
	json wifiList
end

get"/status" do
	status = ["Listening..."]
	json status
end

get '/confirm' do
	sleep(1)
	"true"
end

get '/accessPointMode' do
	sleep(1)
	"Access Point Mode"
end

