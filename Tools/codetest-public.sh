#!/bin/bash
# This file helps freg testing.

FILES="*cpp *.h blocks/*cpp blocks/*.h screens/*cpp screens/*.h everything/everything.cpp"

date +"%T %Y-%m-%d" > report.txt
figlet freg scan >> report.txt
echo >> report.txt
echo 'Number of source files:' >> report.txt
ls $FILES | wc -l >> report.txt

make clean &>> /dev/null
figlet wc >> report.txt
echo >> report.txt
echo "Lines count:" >> report.txt
wc -l $FILES &>> report.txt
flawfinder $FILES | grep Physical >> report.txt
echo >> report.txt
echo "Max lines width:" >> report.txt
wc -L $FILES &>> report.txt

echo 'start cppcheck'
figlet cppcheck >> report.txt
echo >> report.txt
cppcheck --version &>> report.txt
cppcheck --enable=all -q --inconclusive $FILES &>> report.txt
echo 'cppcheck done'

# Lots of irrelevent output.
#figlet flawfinder >> report.txt
#flawfinder $FILES >> report.txt

figlet pmccabe >> report.txt
echo >> report.txt
echo 'top 10 functions:' >> report.txt
pmccabe -vT $FILES >> report.txt
pmccabe -cv $FILES | sort -nr |head -10 >> report.txt

figlet spelling >> report.txt
echo >> report.txt
SPELL_FILES="help_en/* README.md"
SPELL_FILES="$SPELL_FILES $FILES"
LOWERCASE=$(grep -h -o -E '\w{4,}' $SPELL_FILES | sed -e 's/_/ /g' | sed -e 's/[0-9]/ /g' | perl -ne 'print lc(join(" ", split(/(?=[A-Z])/)))' | grep -o -E '\w{4,}')
UPPERCASE=$(grep -h -o -E '[A-Z]{4,}' $SPELL_FILES help_en/* | tr '[:upper:]' '[:lower:]')
WORDS=$(echo "$LOWERCASE" "$UPPERCASE" | sort -u -f | aspell --lang=en_UK --ignore-case list)
if [ -z "$WORDS" ]
	then
		echo "No misspelled words." >> report.txt
	else
		echo "$WORDS" >> report.txt
fi

figlet headers >> report.txt
echo >> report.txt
grep -L "Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm" $FILES >> report.txt

echo 'start scan-build'
figlet scan-build >> report.txt
echo >> report.txt
scan-build make 2>&1 &>>report.txt
make clean &>> /dev/null

echo 'report completed by:' >> report.txt
date +"%T %Y-%m-%d" >> report.txt

