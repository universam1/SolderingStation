use<thread.scad>;

$fn = 50;

$hl = 100;
$h1 = 10.6;
$h2 = 28.6;
$h3 = 14;
$h4 = 12;
$h5 = $hl - ($h1 + $h2 + $h3 + $h4);

module huelle()
{
    difference()
    {
        translate([ 0, 0, 41 ])
            scale([ 0.18, 0.18, 1 ])
            sphere(r = 62);
        translate([ 0, 0, -50 ]) cube(size = [ 100, 100, 100 ], center = true);
        translate([ 0, 0, $hl + 50 ]) cube(size = [ 100, 100, 100 ], center = true);
    }
}

module handle_bottom()
{
    difference()
    {
        union()
        {
            huelle();

            // cylinder(h=$h1+$h2+$h3, r=13/2);
        }
        cylinder(h = $h1, r = 9.8 / 2);
        translate([ 0, 0, $h1 ]) cylinder(h = $h2, r = 10.6 / 2);
        translate([ 0, 0, $h1 + $h2 ]) metric_thread(diameter = 15, pitch = 1.5, length = $h3, internal = true);
        translate([ 0, 0, $h1 + $h2 + $h3 + 50 ]) cube(size = [ 100, 100, 100 ], center = true);
    }
}

module handle_top()
{
    difference()
    {
    union()
    {
        difference()
        {
            huelle();
            translate([ 0, 0, $h1 + $h2 + $h3 - 50 ]) cube(size = [ 100, 100, 100 ], center = true);
        }
        difference()
        {
            translate([ 0, 0, $h1 + $h2 + 2 ]) metric_thread(diameter = 14.4, pitch = 1.5, length = $h3 - 2);
            translate([ 0, 0, $h1 + $h2 ]) cylinder(h = $h3, r = 6.1 / 2);
        }
    }
        // union()
        // {
            // translate([ 0, 0, $h1 + $h2 + $h3 ]) cylinder(h = $h4, r = 12.8 / 2);
            // translate([ 0, 0, $h1 + $h2 + $h3 + $h4 ]) cylinder(h = $h5, r = 12.8 / 2, r2 = 8 / 2);
        
        translate([ 0, 0, $h1 + $h2 + $h3 ]) cylinder(h = $h4 + $h5, r = 4.8 / 2);
    }
}

difference()
{
    union()
    {
        // handle_top();
        handle_bottom();
    }
    // translate([ 0, -15, 0 ]) cube([ 20, 30, 200 ]);
}