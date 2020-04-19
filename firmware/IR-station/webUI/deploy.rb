require 'erb'
require 'zlib'

require 'uglifier'

def jsgz(inpath, outpath)
  Zlib::GzipWriter.open(outpath+".gz") {|gz|
    gz.write Uglifier.compile(File.read(inpath), harmony: true)
  }
end

def gz(inpath, outpath)
  Zlib::GzipWriter.open(outpath+".gz") {|gz|
    gz.write File.read(inpath)
  }
end

def erbgz(inpath, outpath, b)
  Zlib::GzipWriter.open(outpath[%r{([\w\./]+)\.erb}, 1] + ".gz") {|gz|
    gz.write ERB.new(File.read(inpath)).result(b)
  }
end

def search(base, depth, outbase, mappings)
  puts "in #{base+depth}"
  files = `ls -p #{base}#{depth}`.split("\n").sort{|l,r| l[-1]<=>r[-1]}.reverse
  files.each{|file|
    if file[-1] == ?/
      search(base, "#{depth}#{file}", outbase, mappings)
    else
      puts file
      outdir = outbase+depth
      outpath = outdir + file
      inpath = base+depth+file
      `mkdir #{outdir}` if not File.exist?(outdir)

      gzs = mappings.any?{|(pat, func)|
        if inpath =~ pat then
          if func.is_a? Symbol then
            send(func, *[inpath, outpath])
          else
            func.(inpath, outpath)
          end
          true
        else
          false
        end
      }
      `cp #{inpath} #{outpath}` if not gzs
    end
  }
end
