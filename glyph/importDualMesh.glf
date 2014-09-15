# Pointwise V17.2R2 Journal file - Sun Aug 24 22:10:44 2014

package require PWI_Glyph 2.17.2

set pts [list]


#############################################################################
## DualMesh export handler procs
#############################################################################

proc gceVertex { ndx xyz } {
    createPt Gce $ndx $xyz "gcePoint-$ndx"
}


proc vertex { type ndx xyz } {
    global pts
    lappend pts [createPt $type $ndx $xyz "dual${type}Point-$ndx"]
}


proc poly { type indices } {
    global pts
    set polyXyz [list]
    foreach ndx $indices {
	lappend polyXyz [[lindex $pts $ndx] getPoint]
    }
    createPolyCrv $polyXyz $type
    createPolyNormalCrv $polyXyz
}


#############################################################################
## helper procs
#############################################################################

proc setLayerNames { layerArrayVar {desc "layer @key@"} } {
    upvar $layerArrayVar layerArray
    foreach key [array names layerArray] {
	pw::Layer setDescription $layerArray($key) [regsub -all {@key@} $desc $key]
    }
}


proc setAttributes { obj {name false} {color "no_color"} {layer -1} {lineWd 0} } {
    if { false != $name } {
        $obj setName $name
    }
    if { [string is integer -strict $color] } {
        $obj setColor $color
        $obj setRenderAttribute ColorMode Entity
        #$obj setRenderAttribute FillMode Shaded
    } elseif { $color == "cycle" } {
        pw::Entity cycleColors $obj
        $obj setRenderAttribute ColorMode Entity
        #$obj setRenderAttribute FillMode Shaded
    }
    if { $layer >= 0 } {
        $obj setLayer $layer
    }
    if { $lineWd > 0 } {
	$obj setRenderAttribute LineWidth $lineWd
    }
}

set rgbRed      0xff0000
set rgbGreen    0x00ff00
set rgbBlue     0x0000ff
set rgbCyan     0x00ffff
set rgbMagenta  0xff00ff
set rgbYellow   0xffff00
set rgbOrange   0xffa500
set rgbWhite    0xffffff


proc labelPt { ndx xyz {name false} {color ""} {layer -1} {noteHt 0.2} } {
    set note [pw::Note create]
    $note setText "$ndx"
    $note setPosition $xyz
    $note setSize $noteHt
    setAttributes $note "note_$name" $color $layer
    return $note
}


set vertColor(Gce)   $rgbCyan
set vertColor(Bndry) $rgbWhite
set vertColor(Elem)  $rgbOrange
set vertColor(Cnxn)  $rgbYellow
set vertColor(Hard)  $rgbRed

set vertLayer(Gce)	90
set vertLayer(Bndry)	101
set vertLayer(Elem)	100
set vertLayer(Cnxn)	102
set vertLayer(Hard)	103
setLayerNames vertLayer "@key@ dual mesh vertices"


proc createPt { type ndx xyz nm } {
    global vertColor vertLayer
    set pt [pw::Point create]
    $pt setPoint $xyz
    setAttributes $pt $nm $vertColor($type) $vertLayer($type)
    #labelPt $ndx $xyz "${type}Point-$ndx" $vertColor($type) $vertLayer($type) 0.2
    return $pt
}


set polyColor(Bndry)     $rgbMagenta
set polyColor(Interior)  $rgbCyan

set polyLayer(Bndry)     200
set polyLayer(Interior)  201
setLayerNames polyLayer "@key@ dual mesh polygons"

proc createPolyCrv { polyXyz type } {
    global pts polyColor polyLayer
    set anchor [polyCentroid $polyXyz]
    set polyXyz [scaleAboutPt $polyXyz $anchor 0.92]
    # force curve to be closed
    lappend polyXyz [lindex $polyXyz 0]
    return [createCrv $polyXyz "poly${type}-1" $polyLayer($type) $polyColor($type)]
}


proc scaleAboutPt { xyzs anchor scale } {
    set ret {}
    set xform [pwu::Transform scaling -anchor $anchor [list $scale $scale $scale]]
    foreach xyz $xyzs {
	lappend ret [pwu::Transform apply $xform $xyz]
    }
    return $ret
}


proc createPolyNormalCrv { polyXyz } {
    global rgbRed
    set pt0 [polyCentroid $polyXyz]
    set radius [expr {[polyRadius $pt0 $polyXyz] / 2.0}]
    set norm [pwu::Vector3 scale [polyNormal $pt0 $polyXyz] $radius]
    set edge [list $pt0 [pwu::Vector3 add $pt0 $norm]]
    return [createCrv $edge "polyNorm-1" 400 $rgbRed 2]
}

pw::Layer setDescription 400 "Dual Mesh polygon centroid normals"


proc polyCentroid { polyXyz } {
    set ret {0 0 0}
    foreach xyz $polyXyz {
	set ret [pwu::Vector3 add $ret $xyz]
    }
    set ret [pwu::Vector3 divide $ret [llength $polyXyz]]
    return $ret
}


proc polyNormal { pivotXyz polyXyz } {
    set ret {0 0 0}
    lappend polyXyz [lindex $polyXyz 0]
    set len [llength $polyXyz]
    for {set n1 1} {$n1 < $len} {incr n1} {
	set n0 [expr {$n1 - 1}]
	set ret [pwu::Vector3 add $ret \
	    [triNormal $pivotXyz [lindex $polyXyz $n0] [lindex $polyXyz $n1]]]
    }
    set ret [pwu::Vector3 normalize [pwu::Vector3 divide $ret [incr len -1]]]
    return $ret
}


proc polyRadius { pivotXyz polyXyz } {
    set ret 0.0
    foreach xyz $polyXyz {
	set ret [expr {$ret + [pwu::Vector3 length [pwu::Vector3 subtract $pivotXyz $xyz]]}]
    }
    set ret [expr {$ret / [llength $polyXyz]}]
    return $ret
}


proc triNormal { p0 p1 p2 } {
    set e0 [pwu::Vector3 subtract $p1 $p0]
    set e1 [pwu::Vector3 subtract $p2 $p0]
    return [pwu::Vector3 cross $e0 $e1]
}


proc createCrv { xyzs name layer {color #8080ff} {lineWd 1} } {
    set spl [pw::SegmentSpline create]
    foreach xyz $xyzs {
	$spl addPoint $xyz
    }
    set crv [pw::Curve create]
    $crv addSegment $spl
    setAttributes $crv $name $color $layer $lineWd
    unset spl
    return $crv
}

source [file join [file dirname [info script]] DualMeshData.out]
