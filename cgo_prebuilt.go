//go:build lib

// Prebuilt mode: link against prebuilt static library.
// Usage: go build -tags lib
// Requires: prebuilt libfinkit_ds_static.a in build/lib/
package ds

/*
#cgo CFLAGS: -I${SRCDIR}/include
#cgo CFLAGS: -I${SRCDIR}/modules/platform/include
#cgo linux,amd64   LDFLAGS: -L${SRCDIR}/build/linux_amd64 -lfinkit_ds_static -lm
#cgo linux,arm64   LDFLAGS: -L${SRCDIR}/build/linux_arm64 -lfinkit_ds_static -lm
#cgo darwin,amd64  LDFLAGS: -L${SRCDIR}/build/darwin_amd64 -lfinkit_ds_static -lm
#cgo darwin,arm64  LDFLAGS: -L${SRCDIR}/build/darwin_arm64 -lfinkit_ds_static -lm
#cgo windows,amd64 LDFLAGS: -L${SRCDIR}/build/windows_amd64 -lfinkit_ds_static -lm

*/
import "C"
