set -eu
exec=$1
file=$2
outputfile=`tempfile`
$exec < "./in/$file" > $outputfile
cargo run -q --manifest-path ./tools/Cargo.toml --release --bin vis "./in/$file" $outputfile