# ok, wie funktioniert das hier?
# Nunja: Offenbar werden alle Source-Files, bevor sie compilet werden, nochmal kopiert, oder so. kA
# Jedenfalls muss in dem Source-Ordner, den cmake benutzt, und wo halt auch generierte Dateien liegen
# offenbar die ibm_fnt.c liegen
# gelesen wird ${CMAKE_CURRENT_SOURCE_DIR}/ibm.fnt, und die ausgabe geht nach ibm_fnt.c. Wichtig:
# Das Arbeitsverzeichnis ist NICHT ${CMAKE_CURRENT_SOURCE_DIR}, sondern ein anderes Verzeichnis...
# ... ... WO DANN ABER DIE SOURCEN LIEGEN!
# ja. weird. ok.

add_custom_command(OUTPUT "dosbox-8x16.c"
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/gen_sources.sh "${CMAKE_CURRENT_SOURCE_DIR}")


add_library (simplefonts load.c set.c write.c dosbox-8x16.c myarraystream.c)


# include-directory for this library
target_include_directories (simplefonts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
