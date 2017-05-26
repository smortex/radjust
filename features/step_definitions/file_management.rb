require 'securerandom'

Given(/^a file "([^"]*)" exists and is (#{FILESIZE})$/) do |name, size|
  filename = tmp_file_name(name)

  File.open(filename, 'wb') do |f|
    f.write(SecureRandom.random_bytes(size.to_i))
  end
end

Given(/^a file "([^"]*)" does not exist$/) do |name|
  filename = tmp_file_name(name)

  File.unlink(filename) if File.exist?(filename)
end

Then(/^the file "([^"]*)" should exist$/) do |name|
  filename = tmp_file_name(name)

  expect(File.exist?(filename)).to be_truthy
end

Then(/^the file "([^"]*)" sould be (#{FILESIZE}) long$/) do |name, size|
  filename = tmp_file_name(name)

  expect(File.stat(filename).size).to eq(size.to_i)
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
