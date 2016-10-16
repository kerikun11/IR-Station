require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

station = {
	"version"=>"v1.5.0",
	"mode"=>1,
	"hostname"=>"demo",
	"is_stealth_ssid"=>false,
	"ssid"=>"WiFi-2.4GHz",
	"password"=>"pw",
	"is_static_ip"=>false,
	"local_ip"=>151759040,
	"subnetmask"=>16777215,
	"gateway"=>17541312,
	"next_id"=>6,
	"signals"=>[],
	"new_schedule_id"=>0,
	"schedules"=>[]
}

for i in 1..5 do
	station["signals"].push({
		"id"=>i,
		"name"=>"NAME #{i}",
		"path"=>"#{i} NAME#{i}",
		"display"=>true,
		"row"=>i,
		"column"=>i
	})
end

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get"/info" do
	puts params
	sleep(0.2)
	json station
end

post "/signals/send" do
	puts params
	sleep(0.2)
	name  = station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["name"]
	"Sending Successful: #{name}"
end

post "/signals/record" do
	puts params
	sleep(0.5)
	signal = {
		"id"=>station["next_id"],
		"name"=>params[:name],
		"path"=>"/main/#{station["next_id"]} #{params[:name]}.json",
		"display"=>((params[:display]=="true")?true:false),
			"row"=>params[:row].to_i,
			"column"=>params[:column].to_i
	}
	station["next_id"] += 1
	station["signals"].push(signal)
	"Recording Successful: #{params[:name]}"
end

post "/signals/rename" do
	puts params
	station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["name"]=params[:name]
	"Renaming Successful: #{params[:name]}"
end

post "/signals/move" do
	puts params
	station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["row"]=params[:row]
	station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["column"]=params[:column]
	"Renaming Successful: #{params[:name]}"
end

post "/signals/upload" do
	puts params
	signal = {
		"id"=>station["next_id"],
		"name"=>params[:name],
		"path"=>"/main/#{station["next_id"]} #{params[:name]}.json",
		"display"=>((params[:display]=="true")?true:false),
			"row"=>params[:row].to_i,
			"column"=>params[:column].to_i
	}
	station["next_id"] += 1
	station["signals"].push(signal)
	"Uploading Successful: #{params[:name]}"
end

post "/signals/clear" do
	puts params
	station["signals"].delete station["signals"].select{|item| item["id"]==params[:id].to_i}[0]
	"Cleaned"
end

post "/signals/clear-all" do
	puts params
	station["signals"]=[]
	"Cleaned"
end

post "/schedule/new" do
	puts params
	name  = station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["name"]
	schedule = {
		"schedule_id"=>station["new_schedule_id"],
		"id"=>params[:id],
		"time"=>params[:time]
	}
	station["schedules"].push(schedule)
	station["new_schedule_id"] += 1
	"Scheduling Successful: #{name}"
end

get "/schedule/delete" do
	puts params
	schedule = station["schedules"].select{|item| item["schedule_id"]==params[:schedule_id].to_i}[0]
	station["schedules"].delete(schedule)
	"Scheduling Successful"
end

post "/wifi/disconnect" do
	puts params
	sleep(1)
	""
end

=begin
post "/wifi/change-ip" do
	puts params
	sleep(1)
	"Changed IP Address"
end
=end

