require 'open3'

When(/^I synchronize "([^"]*)" \-> "([^"]*)"$/) do |source, destination|
  server_stdin, server_stdout, server_stderr, server_thr = Open3.popen3("./server/server #{tmp_file_name(destination)}")
  server_stdin.close

  port = server_stdout.readline.to_i

  client_stdin, client_stdout, client_stderr, client_thr = Open3.popen3("./client/client #{port} #{tmp_file_name(source)}")
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

When(/^I synchronize local "([^"]*)" \-> remote "([^"]*)"$/) do |source, destination|
  `env PATH=radjust radjust -e fakessh/fakessh #{tmp_file_name(source)} user@host:#{tmp_file_name(destination)}`
  expect($?.success?).to be true
end

When(/^I synchronize remote "([^"]*)" \-> local "([^"]*)"$/) do |source, destination|
  `env PATH=radjust radjust -e fakessh/fakessh user@host:#{tmp_file_name(source)} #{tmp_file_name(destination)}`
  expect($?.success?).to be true
end
