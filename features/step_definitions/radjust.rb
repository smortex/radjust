require 'English'

When(/^I synchronize "([^"]*)" \-> "([^"]*)"$/) do |source, destination|
  File.unlink('socket') if File.exist?('socket')

  server_pid = spawn("./server/server #{tmp_file_name(destination)}")

  sleep 0.001 until File.exist?('socket')

  client_pid = spawn("./client/client #{tmp_file_name(source)}")

  Process.wait(server_pid)
  expect($CHILD_STATUS.success?).to be true

  Process.wait(client_pid)
  expect($CHILD_STATUS.success?).to be true
end
