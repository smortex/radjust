Feature: File adjustement
  Scenario: Adjust file size
    Given a file "source" exists and is 42 bytes
    And a file "target" does not exist
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 bytes long
    And "source" and "target" should have the same mtime

  Scenario: Adjust file size
    Given a file "source" exists and is 42 bytes
    And a file "target" exists and is 42 bytes
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 bytes long
    And "source" and "target" should have the same mtime

  Scenario: Adjust file size
    Given a file "source" exists and is 42 bytes
    And a file "target" exists and is 4096 bytes
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 bytes long
    And "source" and "target" should have the same mtime
