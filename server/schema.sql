CREATE EXTENSION IF NOT EXISTS "pgcrypto";
CREATE EXTENSION IF NOT EXISTS citext;

CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name CITEXT UNIQUE NOT NULL CHECK (name ~ '^[a-zA-Z0-9_]{3,30}$'),
    email CITEXT UNIQUE NOT NULL CHECK (email ~ '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'),
    password_file BYTEA UNIQUE NOT NULL,
    protected_account_key BYTEA UNIQUE NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    version INT NOT NULL DEFAULT 1
);

CREATE TABLE vaults (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    owner_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name TEXT NOT NULL CHECK (name ~ '^[a-zA-Z0-9_]{3,30}$'),
    description TEXT,
    protected_vault_key BYTEA UNIQUE NOT NULL,
    salt BYTEA NOT NULL DEFAULT gen_random_bytes(16),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    version INT NOT NULL DEFAULT 1,
    CONSTRAINT unique_user_vault_name UNIQUE(owner_id, name)
);

CREATE TABLE vault_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    vault_id UUID NOT NULL REFERENCES vaults(id) ON DELETE CASCADE,
    protected_metadata BYTEA NOT NULL,
    protected_payload BYTEA NOT NULL,
    name_hash BYTEA NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    version INT NOT NULL DEFAULT 1,
    CONSTRAINT unique_vault_item_name UNIQUE(vault_id, name_hash)
);

CREATE TABLE user_security_state (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    login_attempts INT DEFAULT 0,
    last_attempt_at TIMESTAMPTZ,
    next_allowed_attempt TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_vaults_owner ON vaults(owner_id);
CREATE INDEX idx_vault_items_vault ON vault_items(vault_id);
CREATE INDEX idx_users_active_email ON users(email) WHERE (is_active = TRUE);
CREATE INDEX idx_users_active_name ON users(name) WHERE (is_active = TRUE);

CREATE OR REPLACE FUNCTION trigger_set_timestamp()
RETURNS TRIGGER AS $$
BEGIN
  NEW.updated_at = NOW();
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION initialize_user_security()
RETURNS TRIGGER AS $$
BEGIN
    INSERT INTO user_security_state (user_id) VALUES (NEW.id);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION reset_expired_lockout()
RETURNS TRIGGER AS $$
BEGIN
    IF OLD.next_allowed_attempt < NOW() AND OLD.login_attempts > 0 THEN
        NEW.login_attempts = 0;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER set_timestamp_users BEFORE UPDATE ON users FOR EACH ROW EXECUTE PROCEDURE trigger_set_timestamp();
CREATE TRIGGER set_timestamp_vaults BEFORE UPDATE ON vaults FOR EACH ROW EXECUTE PROCEDURE trigger_set_timestamp();
CREATE TRIGGER set_timestamp_vault_items BEFORE UPDATE ON vault_items FOR EACH ROW EXECUTE PROCEDURE trigger_set_timestamp();
CREATE TRIGGER set_timestamp_user_security_state BEFORE UPDATE ON user_security_state FOR EACH ROW EXECUTE PROCEDURE trigger_set_timestamp();
CREATE TRIGGER after_user_signup AFTER INSERT ON users FOR EACH ROW EXECUTE PROCEDURE initialize_user_security();
CREATE TRIGGER check_lockout_expiration BEFORE UPDATE ON user_security_state FOR EACH ROW EXECUTE PROCEDURE reset_expired_lockout();