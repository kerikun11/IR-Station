require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

station = {
	"version"=>"v1.4.1",
	"mode"=>1,
	"hostname"=>"demo",
	"is_stealth_ssid"=>false,
	"ssid"=>"WiFi-2.4GHz",
	"password"=>"pw",
	"is_static_ip"=>false,
	"local_ip"=>3232238345,
	"subnetmask"=>0,
	"gateway"=>0,
	"next_id"=>5,
	"signals"=>[]
}

for i in 0...5 do
	station["signals"].push({
		"id"=>i,
		"name"=>"NAME #{i}",
		"path"=>"#{i} NAME#{i}",
		"display"=>true,
		"position"=>{
			"row"=>0,
			"column"=>i
		}
	})
end

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get"/info" do
	sleep(0.2)
	json station
end

post "/signals/send" do
	sleep(0.2)
	name  = station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["name"]
	"Sending Successful: #{name}"
end

post "/signals/record" do
	sleep(0.5)
	signal = {
		"id"=>station["next_id"],
		"name"=>params[:name],
		"path"=>"/main/#{station["next_id"]} #{params[:name]}.json",
		"display"=>((params[:display]=="true")?true:false),
		"position"=>{
			"row"=>params[:row].to_i,
			"column"=>params[:column].to_i
		}
	}
	station["next_id"] += 1
	station["signals"].push(signal)
	"Recording Successful: #{params[:name]}"
end

post "/signals/rename" do
	station["signals"].select{|item| item["id"]==params[:id].to_i}[0]["name"]=params[:name]
	"Renaming Successful: #{params[:name]}"
end

post "/signals/upload" do
end

post "/signals/clear" do
end

post "/signals/clear-all" do
end

post "/wifi/disconnect" do
	sleep(1)
	""
end
=begin
post "/wifi/change-ip" do
	sleep(1)
	"Changed IP Address"
end
=end

