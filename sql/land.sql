CREATE TABLE `land` (
	`LandSetId` INT UNSIGNED NULL,
	`LandId` INT UNSIGNED NULL,
	`Size` SMALLINT NULL,
	`Status` SMALLINT NULL,
	`LandPrice` BIGINT NULL,
	`UpdateTime` BIGINT NULL,
	`OwnerId` BIGINT UNSIGNED NULL,
	`HouseId` BIGINT UNSIGNED NULL,
	`IS_DELETE` SMALLINT NULL DEFAULT '0',
	`IS_NOT_ACTIVE_FLG` SMALLINT NULL DEFAULT '0',
	`UPDATE_DATE` DATETIME NULL
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
