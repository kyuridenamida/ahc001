set -eu
exec=$1
file=$2
$exec < ./in/$file > /tmp/$file.out
cargo run -q --manifest-path ./tools/Cargo.toml --release --bin vis ./in/$file /tmp/$file.out