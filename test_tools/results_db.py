import sqlite3


RESULTS_DB = "test-results.sqlite"


def connect():
    connection = sqlite3.connect(RESULTS_DB)
    initialize(connection)
    return connection


def initialize(connection):
    connection.execute(
        """
        CREATE TABLE IF NOT EXISTS perf (
          job TEXT,
          branch TEXT,
          commit_hash TEXT,
          dirty TEXT,
          test TEXT,
          real REAL,
          user REAL,
          sys REAL,
          status INTEGER,
          result TEXT
        )
        """)
    connection.execute(
        """
        CREATE TABLE IF NOT EXISTS coverage (
          job TEXT,
          branch TEXT,
          commit_hash TEXT,
          dirty TEXT,
          filename TEXT,
          regions INTEGER,
          missed_regions INTEGER,
          region_cover REAL,
          functions INTEGER,
          missed_functions INTEGER,
          function_cover REAL,
          lines INTEGER,
          missed_lines INTEGER,
          line_cover REAL,
          branches INTEGER,
          missed_branches INTEGER,
          branch_cover REAL
        )
        """)
