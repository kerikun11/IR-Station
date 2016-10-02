require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

ch_size_max = 100
ch_size = 25

schedule_list=[]
schedule_id=0

name_list = []
for i in 0...ch_size_max do
	name_list[i]="CH NAME #{i+1}"
end

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get"/signals/list" do
	json name_list[0, ch_size]
end

get"/info" do
	info = {}
	info["message"] = "Loading Successful"
	info["ssid"] = "WiFi-2.4GHz"
	info["ipaddress"]= "192.168.11.6"
	info["hostname"] = "my-room"
	info["version"] = "v1.4.1"
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
	name_list[params[:ch].to_i-1] = params[:name]
	result = {}
	result["code"] = 0
	result["message"] = "Recording Successful"
	json result
end

get "/signals/rename" do
	name_list[params[:ch].to_i-1] = params[:name]
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
	name_list[params[:ch].to_i-1] = ""
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

get "/signals/number" do
	ch_size = params[:number].to_i
	result = {}
	result["code"] = 0
	result["message"] = "Update Successful"
	json result
end

get "/schedule/list" do
	json schedule_list
end

get "/schedule/new" do
	schedule_list += [
		"id"=>schedule_id, "ch"=>params[:ch], "name"=>name_list[params[:ch].to_i-1],"time"=>params[:time]
	]
	schedule_id+=1
	result = {}
	result["code"] = 0
	result["message"] = "Scheduling Successful"
	json result
end

get "/schedule/delete" do
	schedule_list.each do |sch|
		if sch["id"]==params[:id].to_i then
		   schedule_list.delete(sch)
		end
	end
	result = {}
	result["code"] = 0
	result["message"] = "Scheduling Successful"
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

