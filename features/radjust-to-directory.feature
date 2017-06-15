Feature: File adjustement in a directory
  Scenario: Adjust inexistent local file to remote destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    When I synchronize local "source" -> remote "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust inexistent remote file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    When I synchronize remote "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust inexistent local file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    When I synchronize local "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust smaller local file to remote destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 B
    When I synchronize local "source" -> remote "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust smaller remote file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 B
    When I synchronize remote "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust smaller local file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 B
    When I synchronize local "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust larger local file to remote destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 KB
    When I synchronize local "source" -> remote "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust larger remote file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 KB
    When I synchronize remote "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content

  Scenario: Adjust larger local file to local destination directory
    Given a random file "source" exists and is 42 B
    And a directory "target" exist and is empty
    And a random file "target/source" exists and is 41 KB
    When I synchronize local "source" -> local "target"
    Then the file "target/source" should exist
    And the file "target/source" sould be 42 B long
    And "source" and "target/source" should have the same mtime
    And "source" and "target/source" should have the same content
