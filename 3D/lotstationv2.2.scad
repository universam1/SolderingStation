$fn=50;

module object() {
    translate([28.5,32.20,0.25]) {
        import("/Users/waldemar/Projekte/SolderingStation/3D/lotstationv2.1.stl");
    }
    translate([0,52, 13]) color("red") cube([1,12,14]);
}

module holder() {
    union() {
        translate([12,-7.5,14.5]) cylinder(h= 13, r=9.5/2);
        translate([12,-7.5,0]) cylinder(h= 30, r=4/2);
    }
}

difference() {
    object();
    translate([-5,58, 35]) rotate([0,90,0]) cylinder(h= 30, r=7.9/2);
}

difference() {
    translate([5,-15,0]) minkowski() {
        cube([39,16.5,26]);
        cylinder(r=2,h=1);
    }
     
    holder();
    translate([12.5, 0,0]) holder();
    translate([25, 0,0]) holder();
}
