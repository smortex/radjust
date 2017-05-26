require 'English'

When(/^I synchronize "([^"]*)" \-> "([^"]*)"$/) do |source, destination|
  `./server/server #{tmp_file_name(destination)} & ./client/client #{tmp_file_name(source)} & wait`
  expect($CHILD_STATUS.success?).to be_truthy
end
