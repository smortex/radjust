Feature: File adjustement from a directory to a directory
  Background:
    Given a directory "source" exist and is empty
    And a directory "source/b" exist and is empty
    And a random file "source/a" exists and is 42 B
    And a random file "source/b/c" exists and is 42 B
    And a directory "target" exist and is empty

  Scenario: Adjust inexistent local file to remote destination directory
    When I synchronize local "source/" -> remote "target"
    Then the file "target/a" should exist
    And the file "target/a" sould be 42 B long
    And "source/a" and "target/a" should have the same mtime
    And "source/a" and "target/a" should have the same content
    And the file "target/b/c" should exist
    And the file "target/b/c" sould be 42 B long
    And "source/b/c" and "target/b/c" should have the same mtime
    And "source/b/c" and "target/b/c" should have the same content

  Scenario: Adjust inexistent local file to remote destination directory
    When I synchronize local "source" -> remote "target"
    Then the file "target/source/a" should exist
    And the file "target/source/a" sould be 42 B long
    And "source/a" and "target/source/a" should have the same mtime
    And "source/a" and "target/source/a" should have the same content
    And the file "target/source/b/c" should exist
    And the file "target/source/b/c" sould be 42 B long
    And "source/b/c" and "target/source/b/c" should have the same mtime
    And "source/b/c" and "target/source/b/c" should have the same content
