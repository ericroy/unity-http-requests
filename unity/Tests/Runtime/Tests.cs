using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using NUnit.Framework;

using UnityHttpRequests;

namespace Tests
{
	public class Updater
	{
		public bool Done { get; set; }

		private HttpSession session;

		public Updater(HttpSession session)
		{
			this.session = session;
		}

		public void Run(TimeSpan timeout)
		{
			var start = DateTime.Now;
			while (!Done && DateTime.Now - start < timeout)
            {
                session.Update();
                Thread.Sleep(100);
            }
			Assert.IsTrue(Done, "Timed out waiting for response");
		}
	}

    [TestFixture]
    public class TestSuite
    {
        private HttpSession session;

        [SetUp]
        public void Setup()
        {
            session = new HttpSession();
        }

        [TearDown]
        public void TearDown()
        {
            if (session != null)
            {
                session.Dispose();
            }
        }

        [Test]
        public void GetRequest()
        {
            var headers = new Dictionary<string, string>();
            headers["User-Agent"] = "FooAgent";
            headers["Accept"] = "application/json";
            headers["Accept-Encoding"] = "gzip";
            var rid = session.Get("https://httpbin.org/get", headers);

            var updater = new Updater(session);
            session.RequestComplete += (ref Response res) =>
            {
                updater.Done = true;

                TestContext.Out.WriteLine(res.Body.ToStringAlloc());
                
                Assert.AreEqual(rid, res.RequestId, "Request id mismatch");
                Assert.AreEqual(200, res.HttpStatus);

                Assert.IsTrue(res.Headers.HeaderEquals("Content-Type", "application/json"));

                var index = res.Headers.FindHeader("Content-Type");
                Assert.IsTrue(index >= 0);
                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc(index));

                string headerName;
                string headerValue;
                res.Headers.GetHeaderAlloc(index, out headerName, out headerValue);
                Assert.AreEqual("Content-Type", headerName);
                Assert.AreEqual("application/json", headerValue);

                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc("Content-Type"));

                Assert.IsTrue(res.Body.ToStringAlloc().Contains("origin"));
            };

            updater.Run(TimeSpan.FromSeconds(32));
        }

        [Test]
        public void PostRequest()
        {
            var headers = new Dictionary<string, string>();
            headers["User-Agent"] = "FooAgent";
            headers["Accept-Encoding"] = "gzip";
            headers["Accept"] = "application/json";
            headers["Content-Type"] = "application/json";
            var postBody = Encoding.UTF8.GetBytes("{\"foo\": \"bar\", \"baz\": 33}");
            var rid = session.Post("https://httpbin.org/post", postBody, postBody.Length, headers);

            var updater = new Updater(session);
            session.RequestComplete += (ref Response res) =>
            {
                updater.Done = true;
                TestContext.Out.WriteLine(res.Body.ToStringAlloc());
                
                Assert.AreEqual(rid, res.RequestId, "Request id mismatch");
                Assert.AreEqual(200, res.HttpStatus);

                Assert.IsTrue(res.Headers.HeaderEquals("Content-Type", "application/json"));

                var index = res.Headers.FindHeader("Content-Type");
                Assert.IsTrue(index >= 0);
                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc(index));

                string headerName;
                string headerValue;
                res.Headers.GetHeaderAlloc(index, out headerName, out headerValue);
                Assert.AreEqual("Content-Type", headerName);
                Assert.AreEqual("application/json", headerValue);

                Assert.AreEqual("application/json", res.Headers.GetHeaderAlloc("Content-Type"));

                Assert.IsTrue(res.Body.ToStringAlloc().Contains("\"baz\""));
            };

            updater.Run(TimeSpan.FromSeconds(32));
        }
    }
}
