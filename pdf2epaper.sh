#!/bin/bash
pdffile="$1" 
dest="/folder/shared/by/webserver"
src="/spooldir/where/pdfs/end/up"
clean=true

dopdf() {
pdffile="$1"
rm -f $dest/*.png
pages=$(pdfinfo  "$pdffile"  |awk '/Pages:/{print $2;}')
for page in $(seq 1 $pages);do
	pdftoppm -gray -f $page  -scale-to-x 825 -scale-to-y 1200 "$pdffile"|ppmquant 8|pnmtopng  > "$dest/page$page.png"
done
}

inotifywait -m "$src" -e close_write|
while read path action file; do
   dopdf "$path/$file"
   if $clean; then
      rm "$path/$file"
   fi
done
