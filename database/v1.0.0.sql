# Init database

create table users (
  id char(36) not null primary key
);

create table categories (
  id char(36) not null primary key,
  user_id char(36),
  name text not null,
  
  index ix_user_id (user_id),
  
  constraint fk_user_category foreign key (user_id)
    references users(id)
    on delete cascade
    on update restrict,
    
  constraint ch_name_not_empty check (name is null or length(name) > 0)
);

insert into categories (id, user_id, name) values
  ('d0ce1f26-835a-448b-b2ed-932c9e2996d4', null, 'Supermercato'),
  ('a6fac2c5-0c7e-406d-8b7d-517585f2a9c2', null, 'Benzinaio'),
  ('0e3c0251-4363-4b4b-90c9-46962f863f27', null, 'Abbigliamento'),
  ('ce9df353-2dc4-494d-a0e5-8fd772518f72', null, 'Bar'),
  ('519e7e38-fb1d-498c-8b42-67391e3a250d', null, 'Ristorante'),
  ('ce421c83-4d99-4c50-abcf-c7e10b782daf', null, 'Casalinghi'),
  ('21d64468-61d3-455e-ac4e-90baddf540a7', null, 'Animali domestici');

create table receipts (
  id char(36) not null primary key,
  user_id char(36) not null,
  `date` date,
  total_amount decimal(10, 2) not null default 0,
  store_name text not null,
  
  index ix_user_id (user_id),
  
  constraint fk_user_receipt foreign key (user_id)
    references users(id)
    on delete restrict
    on update restrict,
    
  constraint ch_store_name_not_empty check(store_name is null or length(store_name) > 0)
);

create table receipt_items (
  id char(36) not null primary key,
  receipt_id char(36) not null,
  description text not null,
  amount decimal(10, 2) not null default 0,
  category text,
  
  index ix_receipt_id (receipt_id),
  
  constraint fk_receipt_item foreign key (receipt_id)
    references receipts(id)
    on delete cascade
    on update restrict  
);

DELIMITER //

CREATE FUNCTION uuid_v4()
    RETURNS CHAR(36) NO SQL
BEGIN
    -- Generate 8 2-byte strings that we will combine into a UUIDv4
    SET @h1 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');
    SET @h2 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');
    SET @h3 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');
    SET @h6 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');
    SET @h7 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');
    SET @h8 = LPAD(HEX(FLOOR(RAND() * 0xffff)), 4, '0');

    -- 4th section will start with a 4 indicating the version
    SET @h4 = CONCAT('4', LPAD(HEX(FLOOR(RAND() * 0x0fff)), 3, '0'));

    -- 5th section first half-byte can only be 8, 9 A or B
    SET @h5 = CONCAT(HEX(FLOOR(RAND() * 4 + 8)),
                LPAD(HEX(FLOOR(RAND() * 0x0fff)), 3, '0'));

    -- Build the complete UUID
    RETURN LOWER(CONCAT(
        @h1, @h2, '-', @h3, '-', @h4, '-', @h5, '-', @h6, @h7, @h8
    ));
END
//

DELIMITER ;

# 2024-05-09: add sort order to receipt_items
alter table receipt_items 
add column sort_order int not null;

alter table receipt_items 
add unique index ix_receipt_id_sort_order (receipt_id, sort_order);

# 2024-05-12: add request_id and doc_number to receipts
alter table receipts 
add column request_id char(36) not null
after user_id;

alter table receipts 
add column doc_number int not null
after request_id;

alter table receipts 
add constraint doc_number_positive check(doc_number >= 0);

alter table receipts 
add unique index ix_request_id_doc_number (request_id, doc_number);

# 2024-05-14: add currency to receipt_items
alter table receipt_items 
add column currency varchar(3) not null
after amount;

# 2024-05-15: add category to receipt
alter table receipts 
add column category text not null;

# 2024-05-16: add currency to receipt
alter table receipts 
add column currency varchar(3) not null
after total_amount;

# 2024-05-18: remove currency from receipt_items
alter table receipt_items 
drop column currency;

# 2024-06-09: convert receipt request_id into file_name
alter table receipts
change column request_id file_name varchar(100) not null;

alter table receipts
drop index ix_request_id_doc_number;

alter table receipts
add unique index ix_file_name_doc_number (file_name, doc_number);

alter table receipts
drop index ix_file_name_doc_number;

alter table receipts
drop column file_name;

alter table receipts
drop column doc_number;

create table receipt_files (
  id char(36) not null primary key,
  receipt_id char(36) not null,
  file_name varchar(100) not null,
  doc_number int not null,

  unique index ix_receipt_id (receipt_id),
  unique index ix_file_name_doc_number (file_name, doc_number),

  constraint fk_receipt_receipt_file foreign key (receipt_id)
    references receipts(id)
    on delete cascade
    on update restrict,

  constraint ch_file_name_not_empty check(file_name is null or length(file_name) > 0)
);

# 2024-06-10: add state to receipts
alter table receipts
add column state enum('processing', 'done') not null default 'processing';

# v1.1.0
# 2024-07-31: delete default categories
delete from categories where user_id is null;

# 2024-07-31: add budgets table
create table budgets (
  id char(36) not null primary key,
  user_id char(36) not null,
  month date not null,
  amount decimal(10, 2) not null default 0,
  version int not null default 0,

  unique index ix_user_id_month (user_id, month),
  index ix_user_id (user_id),

  constraint fk_user_budget foreign key (user_id)
    references users(id)
    on delete restrict
    on update restrict
);

# 2024-07-31: add color to categories
alter table categories
add column color int not null default 0;

# 2024-07-31: add version to categories and receipts
alter table categories
add column version int not null default 0;

alter table receipts
add column version int not null default 0;

# 2024-07-31: add user devices table
create table user_devices (
  id char(36) not null primary key,
  user_id char(36) not null,

  index ix_user_id (user_id),

  constraint fk_user_device foreign key (user_id)
    references users(id)
    on delete cascade
    on update restrict
);

# 2024-07-31: add entity events table
create table entity_events (
  id char(36) not null primary key,
  device_id char(36) not null,
  entity_type varchar(100) not null,
  entity_id char(36) not null,
  event_type enum('create', 'update', 'delete') not null,
  event_timestamp timestamp not null default current_timestamp,

  index ix_device_id (device_id),

  constraint fk_device_entity_event foreign key (device_id)
    references user_devices(id)
    on delete cascade
    on update restrict
);

# 2024-08-04: drop receipt_files table
drop table receipt_files;

# 2024-08-04: add column image_name to receipts
alter table receipts
add column image_name varchar(100) not null default '';

# 2024-08-04: add index on image_name
alter table receipts
add index ix_receipt_image_name(image_name);

# 2024-08-05: add state 'failed' to receipt state
alter table receipts
drop column state;

alter table receipts
add column state enum('processing', 'done', 'failed') not null default 'processing';

# 2024-08-07: add timestamp and is_deleted to receipts
alter table receipts
add column modified_timestamp timestamp not null default current_timestamp on update current_timestamp;

alter table receipts
add column is_deleted boolean not null default false;

# 2024-08-07: add timestamp and is_deleted to categories
alter table categories
add column modified_timestamp timestamp not null default current_timestamp on update current_timestamp;

alter table categories
add column is_deleted boolean not null default false;

# 2024-08-07: add timestamp to budgets
alter table budgets
add column modified_timestamp timestamp not null default current_timestamp on update current_timestamp;
