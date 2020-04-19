alexa = true
b     = binding
if not ARGV.any?{|e| e =~ /deploy/}
require 'net/http'
require 'uri'

require 'sinatra'

pub = File.dirname(__FILE__) + "/data/main/"
set :public_folder, pub
set :views, File.dirname(__FILE__) + "/#{ARGV[1]}/"

get %r</([a-zA-Z\.]+)\.erb> do
  erb params["captures"].first.to_sym, locals: {alexa: alexa}
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

else

require_relative 'deploy'

f = File.dirname(__FILE__)+?/
mappings = [
  [/\.js$/,  :jsgz],
  [/\.htm$/, :gz],
  [/\.css$/, :gz],
  [/\.erb$/, ->(i,o){erbgz(i,o, b)}],
]

search(f+"data/", "./", f+"/../data/", mappings)
search(f+"views/", "./", f+"/../data/", mappings)

end
