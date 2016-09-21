require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

ch_size_max = 100
ch_size = 25

name = []
for i in 0...ch_size_max do
	name[i]="ch name #{i+1}"
end

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get"/layouts/table" do
	json name[0, ch_size]
end

get"/info" do
	info = {}
	info["message"] = "Loading Successful"
	info["ssid"] = "WiFi-2.4GHz"
	info["ipaddress"]= "192.168.11.6"
	info["hostname"] = "my-room"
	json info
end

get "/signals/send" do
	sleep(0.5)
	result = {}
	result["code"] = 0
	result["message"] = "Sending Successful"
	json result
end

get "/signals/record" do
	sleep(1)
	result = {}
	result["code"] = 0
	result["message"] = "Recording Successful"
	json result
end

get "/signals/rename" do
	result = {}
	result["code"] = 0
	result["message"] = "Renaming Successful"
	json result
end

get "/signals/upload" do
	result = {}
	result["code"] = 0
	result["message"] = "Uploading Successful"
	json result
end

get "/signals/clear" do
	result = {}
	result["code"] = 0
	result["message"] = "Cleaning Successful"
	json result
end

get "/signals/clear-all" do
	sleep(1)
	for i in 0...ch_size_max do
		name[i]=""
	end
	result = {}
	result["code"] = 0
	result["message"] = "Cleaning All Successful"
	json result
end

get "/wifi/disconnect" do
	sleep(1)
	result = {}
	result["code"] = 0
	result["message"] = "Disconnected the WiFi"
	json result
end

get "/wifi/change-ip" do
	sleep(1)
	result = {}
	result["code"] = 0
	result["message"] = "Changed IP"
	json result
end

