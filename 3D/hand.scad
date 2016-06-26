 use <thread.scad>;

$fn = 50;

$h1 = 10.6;
$h2 = 31.2;
$h3 = 13.5;
$h4 = 12;
$h5 = 75-($h1+$h2+$h3+$h4);
    
module handle_bottom() {
    difference() {
        cylinder(h=$h1+$h2+$h3, r=12.8/2);
        cylinder(h=$h1, r=9.2/2);
        translate([0,0,$h1]) cylinder(h=$h2, r=10.6/2);
        translate([0,0,$h1+$h2]) metric_thread (diameter=11, pitch=1, length=$h3, internal=true);
    }
}

module handle_top() {
    difference() {
        translate([0,0,$h1+$h2+2]) metric_thread (diameter=10.4, pitch=1, length=$h3-2);
        translate([0,0,$h1+$h2]) cylinder(h=$h3, r=6.1/2);
    }
    difference() {
        union() {
            translate([0,0,$h1+$h2+$h3]) cylinder(h=$h4, r=12.8/2);
            translate([0,0,$h1+$h2+$h3+$h4]) cylinder(h=$h5, r=12.8/2, r2=8/2);
        }
        translate([0,0,$h1+$h2+$h3]) cylinder(h=$h4+$h5, r=4.8/2);
    }
}


difference() {
    union() {
        handle_bottom();
        handle_top();
    }
    translate([0,-15,0]) cube([10,30,76]);
}