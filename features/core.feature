Feature: File adjustement
  Scenario: Adjust file size
    Given a file "source" exists and is 42 B
    And a file "target" does not exist
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust file size
    Given a file "source" exists and is 42 B
    And a file "target" exists and is 42 B
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content

  Scenario: Adjust file size
    Given a file "source" exists and is 42 B
    And a file "target" exists and is 4 KB
    When I synchronize "source" -> "target"
    Then the file "target" should exist
    And the file "target" sould be 42 B long
    And "source" and "target" should have the same mtime
    And "source" and "target" should have the same content
