Feature: File adjustement
  Scenario: Adjust inexistent local to remote destination file
    Given a random file "source" exists and is 42 B
    And a file "target" does not exist
    When I synchronize local "source" -> remote "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust inexistent remote to local destination file
    Given a random file "source" exists and is 42 B
    And a file "target" does not exist
    When I synchronize remote "source" -> local "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust smaller local to remote destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 41 B
    When I synchronize local "source" -> remote "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust smaller remote to local destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 41 B
    When I synchronize remote "source" -> local "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust larger local to remote destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 41 KB
    When I synchronize local "source" -> remote "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust larger remote to local destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 41 KB
    When I synchronize remote "source" -> local "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

