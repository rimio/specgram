# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.9.2] - 2021-07-22
### Added
- Unit tests.
- Header file comments.

### Removed
- Removed `spdlog` as a dependency due to the `fmt 8.0.0` debacle.

### Fixed
- \[SERIOUS\] Window function term computation ([commit](https://github.com/rimio/specgram/commit/46f0cc9395ffc841626d9242238868383c146233)).
- Axes ticks computation is now much more refined.
- Assertion fail when frequency band and window size are very small.

## [0.9.1] - 2021-07-16
### Added
- Support for `-` as input or output filename, indicating `stdin`/`stdout`.
- Ability to dump the PNG encoded version of output to `stdout`.
- Support for arbitrary scales with custom units.
- Support for linear scales (as opposed to the logarithmic dBFS scale supported in 0.9.0).

### Changed
- Main window is rendered on each iteration of the main loop. Underlying texture is still only updated when new windows are computed.

### Fixed
- \[SERIOUS\] Branch on uninitialized variable ([commit](https://github.com/rimio/specgram/commit/b13609afcdf66d781db70fb75f6869a052a49079)).
- Garbled window texture when no input is received.
- Tick labels no longer overlap; enforce minimum tick spacing based on text width. 
