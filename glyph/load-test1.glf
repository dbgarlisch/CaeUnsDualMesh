# Pointwise V17.2R2

package require PWI_Glyph 2.17.2

set pts [list]

proc vertex { type ndx xyz } {
    global pts
    set pt [pw::Point create]
    $pt setPoint $xyz
    $pt setName "dual${type}Point-$ndx"
    $pt setLayer 100
    lappend pts $pt
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


proc createPolyCrv { polyXyz type } {
    if { $type == "B" } {
	set layer 200
	set color #ff00ff
    } else {
	set layer 300
	set color #00ffff
    }
    # force curve to be closed
    lappend polyXyz [lindex $polyXyz 0]
    return [createCrv $polyXyz "poly${type}-1" $layer $color]
}


proc createPolyNormalCrv { polyXyz } {
    set pt0 [polyCentroid $polyXyz]
    set radius [expr {[polyRadius $pt0 $polyXyz] / 3.0}]
    set norm [pwu::Vector3 scale [polyNormal $pt0 $polyXyz] $radius]
    set edge [list $pt0 [pwu::Vector3 add $pt0 $norm]]
    return [createCrv $edge "polyNorm-1" 400 #f00000]
}


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


proc createCrv { xyzs name layer {color #8080ff} } {
    set spl [pw::SegmentSpline create]
    foreach xyz $xyzs {
	$spl addPoint $xyz
    }
    set crv [pw::Curve create]
    $crv addSegment $spl
    $crv setName $name
    $crv setLayer $layer
    $crv setRenderAttribute ColorMode Entity
    $crv setColor $color
    unset spl
    return $crv
}

source [file join [file dirname [info script]] test1.glf]
