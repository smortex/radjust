require 'English'

When(/^I synchronize "([^"]*)" \-> "([^"]*)"$/) do |source, destination|
  `./client/client #{tmp_file_name(source)} | ./server/server #{tmp_file_name(destination)}`
  expect($CHILD_STATUS.success?).to be_truthy
end
