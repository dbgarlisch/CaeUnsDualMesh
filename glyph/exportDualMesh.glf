package require PWI_Glyph 2.17.2

set scriptDir [file dirname [info script]]

set doms [pw::Grid getAll -type pw::DomainUnstructured]

if { 0 == [llength $doms] } {
    puts "Loading test1.pw for export."
    puts "To export a different grid, load it before running this script."
    pw::Application setUndoMaximumLevels 10
    pw::Application reset
    pw::Application markUndoLevel {Journal Reset}
    pw::Application clearModified
    pw::Application reset -keep Clipboard
    set projMode [pw::Application begin ProjectLoader]
      $projMode initialize [file join $scriptDir test1.pw]
      $projMode setAppendMode false
      $projMode load
    $projMode end
    unset projMode
    pw::Application resetUndoLevels
    pw::Application markUndoLevel {Open}
} else {
    puts "Exporting exisiting grid..."
}

set doms [pw::Grid getAll -type pw::DomainUnstructured]

set ioMode [pw::Application begin CaeExport [pw::Entity sort $doms]]
  $ioMode initialize -type CAE [file join $scriptDir DualMeshData.out]
  if {![$ioMode verify]} {
    error "Data verification failed."
  }
  $ioMode write
$ioMode end
unset ioMode

puts "Dualmesh export completed."
puts "Loading DualMeshData.out..."

source [file join $scriptDir importDualMesh.glf]

puts "All done."
