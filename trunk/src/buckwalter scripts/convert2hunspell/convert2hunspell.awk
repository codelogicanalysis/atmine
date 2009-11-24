function vow(a,b) {
    return vowels==1 ? b : a;
}

BEGIN {
    vowels = 0;
    pfxn = 1000
    needaffix = 1
    circumfix = 2
    acn = 3
    print "90000" >name ".dic" 
    print "SET UTF-8\nFLAG num\nNEEDAFFIX "needaffix"\nCIRCUMFIX " circumfix >name ".aff" 
}
/^;/ {next}
(FILENAME == "stems.0") && (stems[$3] == "") {
    for (i in ac) {
        split(i, t, "\034")
        if (ab[t[1], $3] && bc[$3, t[2]]) {
#            print "Possible prefix-stem-suffix combination: " t[1], $3, t[2]
            if (stems[$3] == "") {
                stems[$3] = "/"needaffix"," ac[i]
            } else {
                stems[$3] = stems[$3] "," ac[i]
            }
        }
    }
    print vow($1,$2) stems[$3]"\t["$3"]" >>"arabic.dic"
    next
}
FILENAME == "stems.0" {
    print vow($1,$2) stems[$3]"\t["$3"]" >>"arabic.dic"
    next
}
FILENAME == "prefixes.0" {
    if (pfx[$3] == "") {
        pfx[$3] = pfxn
        pfxn++
    }
    pfxs[$3] = pfxs[$3] " " vow($1,$2)
    next
}
FILENAME == "suffixes.0" {
    if (sfx[$3] == "") {
        sfx[$3] = 1
    }
    sfxs[$3] = sfxs[$3] " " vow($1,$2)
    next
}
FILENAME=="tableab.0" { ab[$1,$2] = 1 }
FILENAME=="tablebc.0" { bc[$1,$2] = 1 }
FILENAME=="tableac.0" { ac[$1,$2] = acn; acn++ }
END {
    for (i in pfx) {
        n = split(pfxs[i], t)
        print "PFX " pfx[i] " Y " n >>"arabic.aff"
        for (j = 1; j <= n; j++) {
            print "PFX " pfx[i] " 0 " t[j] "/" circumfix " . " i "+" >>"arabic.aff"
        }
    }
    for (i in ac) {
        split(i, t, "\034")
        n = split(sfxs[t[2]], u)
        if (n > 0) {
            print "SFX " ac[i] " Y " n >>"arabic.aff"
            for (j = 1; j <= n; j++) {            
                print "SFX " ac[i] " 0 " u[j] "/" circumfix "," pfx[t[1]]  " . +" t[2] >>"arabic.aff"
            }
        }
    }
}
