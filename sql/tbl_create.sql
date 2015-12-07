CREATE TABLE Info(
       article_id VARCHAR(50),
       article_title LONGTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
       article_summary LONGTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
       png_url LONGTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
       PRIMARY KEY (article_id)
);

CREATE TABLE Category (
       article_id VARCHAR(50),
       article_category VARCHAR(500) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
       FOREIGN KEY (article_id) REFERENCES Info(article_id) ON DELETE CASCADE,
       PRIMARY KEY (article_id, article_category)
);
