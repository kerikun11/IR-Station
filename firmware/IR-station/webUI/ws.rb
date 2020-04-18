require 'net/http'
require 'uri'

require 'sinatra'

pub = File.dirname(__FILE__) + "/main/"
set :public_folder, pub

get %r</([a-zA-Z\.]+)\.erb> do
  erb params["captures"].first.to_sym, locals: {alexa: true}
end

get %r{/([a-zA-Z-]+)(/[a-zA-Z-]+)?} do
  api = params.delete("captures").join
  uri = URI.parse("http://#{ARGV.first}/#{api}")

  req = Net::HTTP::Get.new(uri.path)
  req.set_form_data(params)
  res = Net::HTTP.start(uri.host, uri.port) do |http|
      http.request(req)
  end

  puts "PROXY GET #{uri}, #{params.inspect}"
  puts res.body

  content_type res.content_type
  res.body
end

post %r{/([a-zA-Z-]+)(/[a-zA-Z-]+)?} do
  api = params.delete("captures").join
  uri = URI.parse("http://#{ARGV.first}/#{api}")

  req = Net::HTTP::Post.new(uri)
  req.set_form_data(params)

  res = Net::HTTP.start(uri.hostname, uri.port) do |http|
      http.request(req)
  end

  puts "PROXY POST #{uri}, #{params.inspect}"
  puts res.body

  content_type res.content_type
  res.body
end
