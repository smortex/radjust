Feature: File adjustement
  Scenario: Adjust inexistent destination file
    Given a random file "source" exists and is 42 B
    And a file "target" does not exist
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 78 B
    And the client should have received 1 B
    And the server should have sent 1 B
    And the server should have received 78 B
    And 0 block should have been synchronized
    And 0 chunk should have been synchronized

  Scenario: Adjust smaller destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 2 B
    And "source" and "target" have different mtime
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 111 B
    And the client should have received 34 B
    And the server should have sent 34 B
    And the server should have received 111 B
    And 1 block should have been synchronized
    And 1 chunk should have been synchronized

  Scenario: Adjust different destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 42 B
    And "source" and "target" have different mtime
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 111 B
    And the client should have received 34 B
    And the server should have sent 34 B
    And the server should have received 111 B
    And 1 block should have been synchronized
    And 1 chunk should have been synchronized

  Scenario: Do nothing with synchronized destination file
    Given a random file "source" exists and is 42 B
    And a "target" is synchronized with "source"
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 36 B
    And the client should have received 1 B
    And the server should have sent 1 B
    And the server should have received 36 B
    And 0 block should have been synchronized
    And 0 chunk should have been synchronized

  Scenario: Adjust larger destination file
    Given a random file "source" exists and is 42 B
    And a random file "target" exists and is 4 KB
    And "source" and "target" have different mtime
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 111 B
    And the client should have received 34 B
    And the server should have sent 34 B
    And the server should have received 111 B
    And 1 block should have been synchronized
    And 1 chunk should have been synchronized

  Scenario: Transfer only changed chunks
    Given a random file "source" exists and is 42 MB
    And "target" is a copy of "source" with 1 byte changed
    And "source" and "target" have different mtime
    When I synchronize "source" -> "target"
    Then "source" and "target" should have the same size
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
    And the client should have sent 8330 B
    And the client should have received 131076 B
    And the server should have sent 131076 B
    And the server should have received 8330 B
    And 1 block should have been synchronized
    And 1 chunk should have been synchronized
