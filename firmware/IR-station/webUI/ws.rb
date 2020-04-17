require 'sinatra'

pub = File.dirname(__FILE__) + "/main/"
set :public_folder, pub

get %r</([a-zA-Z\.]+)\.erb> do
  erb params["captures"].first.to_sym, locals: {alexa: true}
end

get '/info' do
  content_type :json

  File.read(pub + "station.json")
end

