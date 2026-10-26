# Production Readiness TODO

## Critical

- [ ] Add authentication for sensitive REST endpoints (`/api/save`, `/api/connect`, `/api/return-to-ap`).
- [ ] Protect AP fallback mode with configurable WPA2 password (avoid open AP in production).
- [ ] Stop storing WiFi credentials as plaintext; add secure-at-rest strategy (device-bound encryption or external secret handling).

## High

- [ ] Remove or sanitize logs that can expose SSID/password or raw request bodies.
- [ ] Align implementation with declared platform support (ESP8266 + ESP32) or narrow metadata until ESP32 path is complete.
- [ ] Replace automatic filesystem format-on-mount-failure with safer recovery flow.
- [ ] Implement `ConfigManager::resetNetwork()` end-to-end.
- [ ] Add request body size limits and reject oversized payloads early.

## Medium

- [ ] Make config persistence atomic (write temp file + fsync/close + rename strategy).
- [ ] Add per-parameter validation hooks (min/max/range/custom validator).
- [ ] Add schema/version migration strategy for config evolution.
- [ ] Add rate limiting/debounce for mutating endpoints to reduce abuse risk.
- [ ] Replace deprecated `StaticJsonDocument` usage with `JsonDocument` where still pending.

## Low

- [ ] Improve operational observability (structured error codes, health states, boot reason reporting).
- [ ] Add metrics/debug endpoint guarded by auth for field diagnostics.
- [ ] Add documentation for secure deployment defaults and onboarding checklist.

## Test and Release

- [ ] Add unit tests for config parsing, type conversions, and edge cases.
- [ ] Add integration tests for REST flow (`config`, `save`, `connect`, `return-to-ap`).
- [ ] Add fault-injection tests for power-loss during save and filesystem mount errors.
- [ ] Define release gate checklist (security checks, tests pass, size budget, manual smoke test).
