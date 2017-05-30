require 'securerandom'

Given(/^a random file "([^"]*)" exists and is (#{FILESIZE})$/) do |name, size|
  filename = tmp_file_name(name)

  File.open(filename, 'wb') do |f|
    f.write(SecureRandom.random_bytes(size.to_i))
    f.fsync
  end
end

Given(/^a file "([^"]*)" does not exist$/) do |name|
  filename = tmp_file_name(name)

  File.unlink(filename) if File.exist?(filename)
end

Given(/^"([^"]*)" is a copy of "([^"]*)" with 1 byte changed$/) do |destination, source|
  source_filename = tmp_file_name(source)
  destination_filename = tmp_file_name(destination)

  FileUtils.cp(source_filename, destination_filename)
  File.open(destination_filename, 'r+') do |f|
    block_size = 16 * 1024 * 1024
    pos = rand(File.size(destination_filename) / block_size * block_size)
    f.seek(pos)
    data = f.readbyte
    data ^= 1
    f.seek(pos)
    f.write(data)
    f.fsync
  end
end

Then(/^the file "([^"]*)" should exist$/) do |name|
  filename = tmp_file_name(name)

  expect(File.exist?(filename)).to be_truthy
end

Then(/^the file "([^"]*)" sould be (#{FILESIZE}) long$/) do |name, size|
  filename = tmp_file_name(name)

  expect(File.stat(filename).size).to eq(size.to_i)
end

Then(/^"([^"]*)" and "([^"]*)" should have the same size$/) do |source, destination|
  source_filename = tmp_file_name(source)
  destination_filename = tmp_file_name(destination)

  expect(File.size(destination_filename)).to eq(File.size(source_filename))
end

Then(/^"([^"]*)" and "([^"]*)" should have the same mtime$/) do |source, destination|
  source_filename = tmp_file_name(source)
  destination_filename = tmp_file_name(destination)

  expect(File.mtime(destination_filename)).to eq(File.mtime(source_filename))
end

Then(/^"([^"]*)" and "([^"]*)" should have the same content$/) do |source, destination|
  source_filename = tmp_file_name(source)
  destination_filename = tmp_file_name(destination)

  expect(Digest::SHA256.hexdigest(File.read(destination_filename))).to eq(Digest::SHA256.hexdigest(File.read(source_filename)))
end

Then(/^the client should have sent (#{FILESIZE})$/) do |n|
  expect(@client_stdout).to match(/sent #{n} bytes/)
end

Then(/^the client should have received (#{FILESIZE})$/) do |n|
  expect(@client_stdout).to match(/received #{n} bytes/)
end
