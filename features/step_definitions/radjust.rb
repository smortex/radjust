require 'open3'

When(/^I synchronize "([^"]*)" \-> "([^"]*)"$/) do |source, destination|
  File.unlink('socket') if File.exist?('socket')

  server_stdin, server_stdout, server_stderr, server_thr = Open3.popen3("./server/server #{tmp_file_name(destination)}")
  server_stdin.close

  sleep 0.001 until File.exist?('socket')

  client_stdin, client_stdout, client_stderr, client_thr = Open3.popen3("./client/client #{tmp_file_name(source)}")
  client_stdin.close

  exit_status = server_thr.value
  expect(exit_status.success?).to be true

  @server_stdout = server_stdout.read
  @server_stderr = server_stderr.read
  server_stdout.close
  server_stderr.close

  exit_status = client_thr.value
  expect(exit_status.success?).to be true

  @client_stdout = client_stdout.read
  @client_stderr = client_stderr.read
  client_stdout.close
  client_stderr.close
end
