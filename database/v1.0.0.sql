create table users (
  id varchar(36) not null primary key
);

create table categories (
  id varchar(36) not null primary key,
  user_id varchar(36),
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
  id varchar(36) not null primary key,
  user_id varchar(36) not null,
  `date` date,
  total_amount decimal(10, 2) not null default 0,
  store_name text not null,
  status enum('processing', 'processed'),
  
  index ix_user_id (user_id),
  
  constraint fk_user_receipt foreign key (user_id)
    references users(id)
    on delete restrict
    on update restrict,
    
  constraint ch_store_name_not_empty check(store_name is null or length(store_name) > 0)
);

create table receipt_item (
  id varchar(36) not null primary key,
  receipt_id varchar(36) not null,
  description text not null,
  amount decimal(10, 2) not null default 0,
  category_id varchar(36),
  
  index ix_receipt_id (receipt_id),
  index ix_category_id (category_id),
  
  constraint fk_receipt_item foreign key (receipt_id)
    references receipts(id)
    on delete cascade
    on update restrict,
    
  constraint fk_receipt_item_category foreign key (category_id)
    references categories(id)
    on delete restrict
    on update restrict    
);
