require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

name = []
for i in 0...25 do
	name[i]="ch #{i+1}"
end

get '/' do
	send_file File.join(settings.public_folder,'index.htm')
end

get "/recode" do
	sleep(1)
	logger.info "ch = "+params[:ch]
	logger.info "name = "+params[:name]
	name[params[:ch].to_i-1]=params[:name]
	json "Recode a signal ch"+params[:ch]+" as "+name[params[:ch].to_i-1]
end

get "/rename" do
	#sleep(1)
	logger.info "ch = "+params[:ch]
	logger.info "name = "+params[:name]
	name[params[:ch].to_i-1]=params[:name]
	json "Rename a signal ch"+params[:ch]+" as "+name[params[:ch].to_i-1]
end

get "/upload" do
	#sleep(2)
	logger.info "ch = "+params[:ch]
	logger.info "name = "+params[:name]
	name[params[:ch].to_i-1]=params[:name]
	json "Rename a signal ch"+params[:ch]+" as "+name[params[:ch].to_i-1]
end

get "/clear" do
	#sleep(2)
	logger.info "ch = "+params[:ch]
	name[params[:ch].to_i-1]="ch #{params[:ch]}"
	json "Cleared a signal ch"+params[:ch]
end

get "/send" do
	#sleep(1)
	logger.info "ch = "+params[:ch]
	json "Sent a signal ch"+params[:ch]+": "+name[params[:ch].to_i-1]
end

get "/clear-all" do
	sleep(1)
	for i in 0...25 do
		name[i]="ch #{i+1}"
	end
	json "Cleared"
end

get "/disconnect-wifi" do
	sleep(1)
	json "Disconnected"
end

get"/name-list" do
	#sleep(1)
	json name
end

get"/info" do
	#sleep(1)
	info = ["Loading Successful","WiFi-2.4GHz","192.168.11.8","http://esp8266.local"]
	json info
end

