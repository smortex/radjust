Feature: File adjustement
  Scenario: Adjust inexistent destination file
    Given a random file "source" exists and is 42 B
    And a file "target" does not exist
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 42 B
    And the client should have received 1 B

  Scenario: Adjust different destination file
    Given a random file "source" exists and is 42 B
    And I wait 0.1 s
    And a random file "target" exists and is 42 B
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 75 B
    And the client should have received 34 B

  Scenario: Adjust larger destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 4 KB
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 75 B
    And the client should have received 34 B

  Scenario: Transfer only changed chunks
    Given a random file "source" exists and is 42 MB
    And "target" is a copy of "source" with 1 byte changed
    When I synchronize "source" -> "target"
    Then "source" and "target" should have the same size
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 8288 B
    And the client should have received 131076 B
